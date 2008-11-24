/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImageSearchInfo.h"
#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>
#include "Settings/SettingsData.h"
#include <klocale.h>
#include <kdebug.h>
#include "CategoryMatcher.h"
#include "ImageDB.h"
#include "ImageInfo.h"
#include <kapplication.h>
#include <kconfig.h>
#include <config-kpa-exiv2.h>
#include <kglobal.h>
#include <kconfiggroup.h>
using namespace DB;

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description )
    : _date( date), _label( label ), _description( description ), _isNull( false ), _compiled( false )
{
}

QString ImageSearchInfo::label() const
{
    return _label;
}

QString ImageSearchInfo::description() const
{
    return _description;
}

ImageSearchInfo::ImageSearchInfo()
    : _isNull( true ), _compiled( false )
{
}

bool ImageSearchInfo::isNull() const
{
    return _isNull;
}

bool ImageSearchInfo::match( ImageInfoPtr info ) const
{
    if ( _isNull )
        return true;

    if ( !_compiled )
        compile();

    bool ok = true;
#ifdef HAVE_EXIV2
    ok = _exifSearchInfo.matches( info->fileName(DB::AbsolutePath) );
#endif

    if ( !_date.start().isNull() ) {
        // Date
        // the search date matches the actual date if:
        // actual.start <= search.start <= actuel.end or
        // actual.start <= search.end <=actuel.end or
        // search.start <= actual.start and actual.end <= search.end

        QDateTime actualStart = info->date().start();
        QDateTime actualEnd = info->date().end();
        if ( actualEnd <= actualStart )  {
            QDateTime tmp = actualStart;
            actualStart = actualEnd;
        actualEnd = tmp;
        }

        bool b1 =( actualStart <= _date.start() && _date.start() <= actualEnd );
        bool b2 =( actualStart <= _date.end() && _date.end() <= actualEnd );
        bool b3 = ( _date.start() <= actualStart && actualEnd <= _date.end() );

        ok &= ( ( b1 || b2 || b3 ) );
    }

    // -------------------------------------------------- Options
    info->clearMatched();
    Q_FOREACH(CategoryMatcher* optionMatcher, _optionMatchers) {
        ok &= optionMatcher->eval( info );
    }


    // -------------------------------------------------- Label
    ok &= ( _label.isEmpty() || info->label().indexOf(_label) != -1 );

    // -------------------------------------------------- Text
    QString txt = info->description();
    if ( !_description.isEmpty() ) {
        QStringList list = _description.split(QChar::fromLatin1(' '), QString::SkipEmptyParts);
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            ok &= ( txt.indexOf( *it, 0, Qt::CaseInsensitive ) != -1 );
        }
    }

    return ok;
}


QString ImageSearchInfo::option( const QString& name ) const
{
    return _options[name];
}

void ImageSearchInfo::setOption( const QString& name, const QString& value )
{
    _options[name] = value;
    _isNull = false;
    _compiled = false;
}

void ImageSearchInfo::addAnd( const QString& category, const QString& value )
{
    QString val = option( category );
    if ( !val.isEmpty() )
        val += QString::fromLatin1( " & " ) + value;
    else
        val = value;

    setOption( category, val );
    _isNull = false;
    _compiled = false;
}

QString ImageSearchInfo::toString() const
{
    QString res;
    bool first = true;
    for( QMap<QString,QString>::ConstIterator it= _options.begin(); it != _options.end(); ++it ) {
        if ( ! it.value().isEmpty() ) {
            if ( first )
                first = false;
            else
                res += QString::fromLatin1( " / " );

            QString txt = it.value();
            if ( txt == ImageDB::NONE() )
                txt = i18nc( "As in No persons, no locations etc. I do realize that translators may have problem with this, "
                            "but I need some how to indicate the category, and users may create their own categories, so this is "
                            "the best I can do - Jesper.", "No %1" ).arg( it.key() );

            if ( txt.contains( QString::fromLatin1("|") ) )
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " %1 " ).arg( i18n("and") ) );

            else
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " / " ) );

            txt.replace( QString::fromLatin1( "|" ), QString::fromLatin1( " %1 " ).arg( i18n("or") ) );
            txt.replace( QString::fromLatin1( "!" ), QString::fromLatin1( " %1 " ).arg( i18n("not") ) );
            txt.replace( ImageDB::NONE(), i18nc( "As in no other persons, or no other locations. "
                                                "I do realize that translators may have problem with this, "
                                                "but I need some how to indicate the category, and users may create their own categories, so this is "
                                                "the best I can do - Jesper.", "No other %1" ).arg( it.key() ) );
            txt.simplified();
            res += txt;
        }
    }
    return res;
}

void ImageSearchInfo::debug()
{
    for( QMap<QString,QString>::Iterator it= _options.begin(); it != _options.end(); ++it ) {
        kDebug() << it.key() << ", " << it.value();
    }
}

// PENDING(blackie) move this into the Options class instead of having it here.
void ImageSearchInfo::saveLock() const
{
    KConfigGroup config = KGlobal::config()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings"));
    config.writeEntry( QString::fromLatin1("label"), _label );
    config.writeEntry( QString::fromLatin1("description"), _description );
    config.writeEntry( QString::fromLatin1("categories"), _options.keys() );
    for( QMap<QString,QString>::ConstIterator it= _options.begin(); it != _options.end(); ++it ) {
        config.writeEntry( it.key(), it.value() );
    }
    config.sync();
}

ImageSearchInfo ImageSearchInfo::loadLock()
{
    KConfigGroup config = KGlobal::config()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings" ));
    ImageSearchInfo info;
    info._label = config.readEntry( "label" );
    info._description = config.readEntry( "description" );
    QStringList categories = config.readEntry<QStringList>( QString::fromLatin1("categories"), QStringList() );
    for( QStringList::ConstIterator it = categories.constBegin(); it != categories.constEnd(); ++it ) {
        info.setOption( *it, config.readEntry<QString>( *it, QString() ) );
    }
    return info;
}

ImageSearchInfo::ImageSearchInfo( const ImageSearchInfo& other )
{
    _date = other._date;
    _options = other._options;
    _label = other._label;
    _description = other._description;
    _isNull = other._isNull;
    _compiled = false;
#ifdef HAVE_EXIV2
    _exifSearchInfo = other._exifSearchInfo;
#endif
}

void ImageSearchInfo::compile() const
{
#ifdef HAVE_EXIV2
    _exifSearchInfo.search();
#endif
    deleteMatchers();

    for( QMap<QString,QString>::ConstIterator it = _options.begin(); it != _options.end(); ++it ) {
        QString category = it.key();
        QString matchText = it.value();

        QStringList orParts = matchText.split(QString::fromLatin1("|"), QString::SkipEmptyParts);
        OptionContainerMatcher* orMatcher = new OptionOrMatcher;

        for( QStringList::Iterator itOr = orParts.begin(); itOr != orParts.end(); ++itOr ) {
            QStringList andParts = (*itOr).split(QString::fromLatin1("&"), QString::SkipEmptyParts);

            OptionContainerMatcher* andMatcher = orMatcher;
            if ( andParts.count() > 1 ) {
                andMatcher = new OptionAndMatcher;
                orMatcher->addElement( andMatcher );
            }


            for( QStringList::Iterator itAnd = andParts.begin(); itAnd != andParts.end(); ++itAnd ) {
                QString str = *itAnd;
                bool negate = false;
                static QRegExp regexp( QString::fromLatin1("^\\s*!\\s*(.*)$") );
                if ( regexp.exactMatch( str ) )  {
                    negate = true;
                    str = regexp.cap(1);
                }
                str = str.trimmed();
                CategoryMatcher* valueMatcher;
                if ( str == ImageDB::NONE() )
                    valueMatcher = new OptionEmptyMatcher( category, !negate );
                else
                    valueMatcher = new OptionValueMatcher( category, str, !negate );
                andMatcher->addElement( valueMatcher );
            }
        }
        if ( orMatcher->_elements.count() == 1 )
            _optionMatchers.append( orMatcher->_elements[0] );
        else if ( orMatcher->_elements.count() > 1 )
            _optionMatchers.append( orMatcher );
    }
    _compiled = true;
}

ImageSearchInfo::~ImageSearchInfo()
{
    deleteMatchers();
}

void ImageSearchInfo::debugMatcher() const
{
    if ( !_compiled )
        compile();

    qDebug("And:");
    Q_FOREACH(CategoryMatcher* optionMatcher, _optionMatchers) {
        optionMatcher->debug(1);
    }
}

QList<QList<OptionSimpleMatcher*> > ImageSearchInfo::query() const
{
    if ( !_compiled )
        compile();

    // Combine _optionMachers to one list of lists in Disjunctive
    // Normal Form and return it.

    QList<CategoryMatcher*>::Iterator it  = _optionMatchers.begin();
    QList<QList<OptionSimpleMatcher*> > result;
    if ( it == _optionMatchers.end() )
        return result;

    result = convertMatcher( *it );
    ++it;

    for( ; it != _optionMatchers.end(); ++it ) {
        QList<QList<OptionSimpleMatcher*> > current = convertMatcher( *it );
        QList<QList<OptionSimpleMatcher*> > oldResult = result;
        result.clear();

        Q_FOREACH(QList<OptionSimpleMatcher*> resultIt, oldResult) {
            Q_FOREACH(QList<OptionSimpleMatcher*> currentIt, current) {
                QList<OptionSimpleMatcher*> tmp;
                tmp += resultIt;
                tmp += currentIt;
                result.append( tmp );
            }
        }
    }
    return result;
}

Q3Dict<void> ImageSearchInfo::findAlreadyMatched( const QString &group ) const
{
    Q3Dict<void> map;
    QString str = option( group );
    if ( str.contains( QString::fromLatin1( "|" ) ) ) {
        return map;
    }

    QStringList list = str.split(QString::fromLatin1( "&" ), QString::SkipEmptyParts);
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString nm = (*it).trimmed();
        if (! nm.contains( QString::fromLatin1( "!" ) ) )
            map.insert( nm, (void*) 0x1 /* something different from 0x0 */ );
    }
    return map;
}

void ImageSearchInfo::deleteMatchers() const
{
    Q_FOREACH(CategoryMatcher* matcher, _optionMatchers) {
        delete matcher;
    }
    _optionMatchers.clear();
}

QList<OptionSimpleMatcher*> ImageSearchInfo::extractAndMatcher( CategoryMatcher* matcher ) const
{
    QList<OptionSimpleMatcher*> result;

    OptionAndMatcher* andMatcher;
    OptionSimpleMatcher* simpleMatcher;

    if ( ( andMatcher = dynamic_cast<OptionAndMatcher*>( matcher ) ) ) {
        Q_FOREACH(CategoryMatcher* child, andMatcher->_elements) {
            OptionSimpleMatcher* simpleMatcher = dynamic_cast<OptionSimpleMatcher*>( child );
            Q_ASSERT( simpleMatcher );
            result.append( simpleMatcher );
        }
    }
    else if ( ( simpleMatcher = dynamic_cast<OptionSimpleMatcher*>( matcher ) ) )
        result.append( simpleMatcher );
    else
        Q_ASSERT( false );

    return result;
}

/** Convert matcher to Disjunctive Normal Form.
 *
 * @return OR-list of AND-lists. (e.g. OR(AND(a,b),AND(c,d)))
 */
QList<QList<OptionSimpleMatcher*> > ImageSearchInfo::convertMatcher( CategoryMatcher* item ) const
{
    QList<QList<OptionSimpleMatcher*> > result;
    OptionOrMatcher* orMacther;

    if ( ( orMacther = dynamic_cast<OptionOrMatcher*>( item ) ) ) {
        Q_FOREACH(CategoryMatcher* child, orMacther->_elements) {
            result.append( extractAndMatcher( child ) );
        }
    }
    else
        result.append( extractAndMatcher( item ) );
    return result;
}

ImageDate ImageSearchInfo::date() const
{
    return _date;
}

#ifdef HAVE_EXIV2
void ImageSearchInfo::addExifSearchInfo( const Exif::SearchInfo info )
{
    _exifSearchInfo = info;
    _isNull = false;
}
#endif

void DB::ImageSearchInfo::renameCategory( const QString& oldName, const QString& newName )
{
    _options[newName] = _options[oldName];
    _options.remove( oldName );
    _compiled = false;
}
