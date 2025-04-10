#!/bin/bash

# SPDX-FileCopyrightText: 2018 - 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

export LC_ALL=C.UTF-8
myname=`basename "$0"`
mydir=`dirname "$0"`

DEPS="kphotoalbum convert exiv2 kdialog"
# basic kphotoalbumrc options to make testing less annoying (i.e. prevent some pop-ups):
BASE_RC="\n[Thumbnails]\ndisplayCategories=true\n[Notification Messages]\nimage_config_typein_show_help=false\n[TipOfDay]\nRunOnStart=false"
TEMPDIR=
KEEP_TEMPDIR=0
declare -A _checks _context _check_db_file

result_ok=0
result_failed=1
result_err_crash=2
result_err_setup=3
result_err_other=4

declare -A LOG_LEVELS
LOG_LEVELS[debug]=0
LOG_LEVELS[info]=1
LOG_LEVELS[notice]=2
LOG_LEVELS[warning]=3
LOG_LEVELS[err]=4
# default log level:
LOG_LEVEL=2

### functions

cleanup()
{
	if [[ "$KEEP_TEMPDIR" == 1 ]]
	then
		log info "NOT removing temporary directory '$TEMPDIR'."
	else
		if [[ -d "$TEMPDIR" ]]
		then
			log debug "Removing '$TEMPDIR'..."
			rm -rf "$TEMPDIR"
		fi
	fi
}

log()
{
	lvl="$1"
	shift
	if [ "${LOG_LEVELS[$lvl]}" -ge "$LOG_LEVEL" ]
	then
		echo "$myname[$lvl]: $*" >&2
	fi
}

print_help()
{
	echo "Usage: $myname [--check] [PARAMETERS...] [--all|CHECKS...]" >&2
	echo "       $myname --help" >&2
	echo "       $myname --list" >&2
	echo "       $myname --print" >&2
	echo "" >&2
	echo "List or run integration tests for KPhotoAlbum." >&2
	echo "This script allows guided integration tests that present the user with concrete things to check." >&2
	echo "" >&2
	echo "Modes:" >&2
	echo "-c|--check                                  Run the specified checks." >&2
	echo "-l|--list                                   List available checks." >&2
	echo "-p|--print                                  Print available checks with description." >&2
	echo "" >&2
	echo "Parameters:" >&2
	echo "--all                                       Run all tests (only valid for --check)." >&2
	echo "--automatic                                 Run tests non-interactively." >&2
	echo "--keep-tempdir                              Do not remove temporary files." >&2
	echo "--log-level debug|info|notice|warning|err   Set log level (default: notice)." >&2
	echo "--tempdir DIR                               Use DIR for temporary files (implies --keep-tempdir)." >&2
	echo "" >&2
}

setup_check()
# setup_check DIR
# sets up a demo db in DIR/db
{
	local check_dir="$1"
	if ! mkdir "$check_dir" "$check_dir/db" "$check_dir/QtProject"
	then
		log err "Could not create check directories for prefix '$check_dir'!"
		return 1
	fi

	# set logging rules
	cat > "$check_dir/QtProject/qtlogging.ini" <<EOF
[Rules]
*=false
kphotoalbum.*=true
EOF

	# copy demo database (except movie.avi)
	# ... movie.avi often makes problems with thumbnailing, causing kphotoalbum to add a token.
	# ... Since this does not happen reproducibly for all platforms, better not add the movie file as long as it isn't required for a test.
	if ! cp -r "$mydir/../demo/"*.jpg "$mydir/../demo/index.xml" "$check_dir/db"
	then
		log err "Could not copy demo database to '$check_dir/db'!"
		return 1
	fi
}

do_list()
{
	for check in "${!_checks[@]}"
	do
		echo "$check"
	done | sort
}

do_print()
{
	for check in "${!_checks[@]}"
	do
		echo "$check -- ${_checks[$check]}"
	done | sort
}

do_checks()
{
	let num_total=0
	let num_ok=0
	let num_failed=0
	let num_err_crash=0
	let num_err_setup=0
	let num_err_other=0
	local names_failed=
	local names_err_crash=
	local names_err_setup=
	local names_err_other=

	for name
	do
		let num_total++
		do_check "$name"
		case "$?" in
			$result_ok)
				log info "$name: OK"
				let num_ok++
				;;
			$result_failed)
				log info "$name: FAILED"
				let num_failed++
				names_failed="$names_failed $name"
				;;
			$result_err_crash)
				log info "$name: ERROR (crash)"
				let num_err_crash++
				names_err_crash="$names_err_crash $name"
				;;
			$result_err_setup)
				log info "$name: ERROR (setup failed)"
				let num_err_setup++
				names_err_setup="$names_err_setup $name"
				;;
			$result_err_other)
				log info "$name: ERROR (undetermined error)"
				let num_err_other++
				names_err_other="$names_err_other $name"
				;;
			*)
				log err "Internal error: invalid return code while running '$name'!"
				exit 1
		esac
	done

	log notice "Summary: $num_ok of $num_total OK, $num_failed failed, $(( num_err_crash + num_err_setup + num_err_other)) errors."
	log notice "Failed: $names_failed"
	log notice "Crashed: $names_err_crash"
	log notice "Setup error: $names_err_setup"

	# return ok if no test failed:
	test "$num_total" -eq "$num_ok"
}

do_check()
{
	local check_name="$1"
	local check_desc="${_checks[$check_name]}"
	if [ -n "$check_desc" ]
	then
		log info "Running check $check_name ($check_desc)..."
		"$check_name"
	else
		log err "No check named '$check_name'!"
		exit 1
	fi
}

generic_check()
# generic_check TESTNAME
# Runs the generic check workflow:
# 1. Prepare files for the test (setup_check, prepare_TESTNAME)
# 2. Execute kphotoalbum via call_TESTNAME
# 3. Check index.xml against reference, if available
# 4. Otherwise ask the user to verify manually.
#
# generic_check expects the prepare_TESTNAME and call_TESTNAME functions to be defined.
{
	local check_name="$1"
	local check_dir="$TEMPDIR/$check_name"
	setup_check "$check_dir" || return $result_err_setup
	if [ -z "$NON_INTERACTIVE" ]
	then
		kdialog --msgbox "<h1>$check_name</h1>${_context[$check_name]}"
	fi
	export XDG_CONFIG_HOME="$check_dir"
	prepare_$check_name "$check_dir"
	call_$check_name "$check_dir" > "$check_dir/log" 2>&1 || return $result_err_crash
	local check_db_file="$mydir/${_check_db_file[$check_name]}"
	if [[ -n "$check_db_file" ]]
	then
		test -f "$check_db_file" || echo "$check_db_file does not exist!"
		if ! diff -u "$check_db_file" "$check_dir/db/index.xml"
		then
			log notice "$check_name: Mismatch in index.xml!"
			return $result_failed
		else
			return $result_ok
		fi
	fi
	# fallback: ask the user to verify
	if [ -n "$NON_INTERACTIVE" ]
	then
		log err "$check_name: Error - test result could not be determined automatically!"
		return $result_err_other
	fi
	if kdialog --yesno "<h1>$check_name &mdash; Did KPhotoAlbum pass the test?</h1><p>As a reminder what you should check:</p><hr/><div style='text-size=small'>${_context[$check_name]}</div>"
	then
		return $result_ok
	else
		log notice "$check_name: Failed test as determined by user."
		return $result_failed
	fi
}


### MAIN

for dep in $DEPS
do
	if ! command -v "$dep" >/dev/null
	then
		log err "Could not find required dependency '$dep'!"
		exit 2
	fi
done

version=`kphotoalbum --version 2>&1`

TEMP=`getopt -o clhp --long "all,automatic,check,help,keep-tempdir,list,log-level:,print,tempdir:" -n "$myname" -- "$@"`
if [ $? != 0 ] ; then log err "Terminating..." ; exit 1 ; fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

MODE=check
while true ; do
	case "$1" in
		-h|--help) print_help ; exit ;;
		-l|--list) MODE="list" ; shift ;;
		-c|--check) MODE="check" ; shift ;;
		-p|--print) MODE="print" ; shift ;;
		--keep-tempdir) KEEP_TEMPDIR=1 ; shift ;;
		--tempdir) TEMPDIR="$2" ; KEEP_TEMPDIR=1 ; shift 2 ;;
		--all) RUN_ALL=1 ; shift ;;
		--automatic) export NON_INTERACTIVE=1 ; shift ;;
		--log-level) LOG_LEVEL="${LOG_LEVELS[$2]}" ; shift 2 ;;
		--) shift ; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done
log info "Using $version (`command -v kphotoalbum`)..."

if [ -z "$TEMPDIR" ]
then
	TEMPDIR=`mktemp -d --tmpdir kphotoalbum-tests-XXXXXX`
fi

trap cleanup EXIT

# read test files
for f in "$mydir/integration-tests/"*.sh
do
	. "$f"
done

case $MODE in
	list|print)
		do_$MODE
		;;
	check)
		if [[ "$RUN_ALL" == 1 ]]
		then
			eval set -- "${!_checks[@]}"
		fi
		do_checks "$@"
		;;
esac
