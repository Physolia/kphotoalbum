/* Copyright (C) 2020 The KPhotoAlbum development team

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

#include "Logging.h"
#include "ThumbnailCacheConverter.h"
#include "version.h"

#include <ImageManager/ThumbnailCache.h>
#include <Settings/SettingsData.h>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QLoggingCategory>
#include <QTextStream>
#include <QTimer>

using namespace KPAThumbnailTool;

int main(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("kphotoalbum");
    QCoreApplication app(argc, argv);

    KAboutData aboutData(
        QStringLiteral("kpa-thumbnailtool"), //component name
        i18n("KPhotoAlbum Thumbnail Tool"), // display name
        QStringLiteral(KPA_VERSION),
        i18n("Tool for inspecting and editing the KPhotoAlbum thumbnail cache"), // short description
        KAboutLicense::GPL,
        i18n("Copyright (C) 2020 The KPhotoAlbum Development Team"), // copyright statement
        QString(), // other text
        QStringLiteral("https://www.kphotoalbum.org") // homepage
    );
    aboutData.setOrganizationDomain("kde.org");
    // maintainer is expected to be the first entry
    // Note: I like to sort by name, grouped by active/inactive;
    //       Jesper gets ranked with the active authors for obvious reasons
    aboutData.addAuthor(i18n("Johannes Zarl-Zierl"), i18n("Development, Maintainer"), QStringLiteral("johannes@zarl-zierl.at"));
    aboutData.addAuthor(i18n("Robert Krawitz"), i18n("Development, Optimization"), QStringLiteral("rlk@alum.mit.edu"));
    aboutData.addAuthor(i18n("Tobias Leupold"), i18n("Development, Releases, Website"), QStringLiteral("tobias.leupold@gmx.de"));
    aboutData.addAuthor(i18n("Jesper K. Pedersen"), i18n("Former Maintainer, Project Creator"), QStringLiteral("blackie@kde.org"));
    // not currently active:
    aboutData.addAuthor(i18n("Hassan Ibraheem"), QString(), QStringLiteral("hasan.ibraheem@gmail.com"));
    aboutData.addAuthor(i18n("Jan Kundr&aacute;t"), QString(), QStringLiteral("jkt@gentoo.org"));
    aboutData.addAuthor(i18n("Andreas Neustifter"), QString(), QStringLiteral("andreas.neustifter@gmail.com"));
    aboutData.addAuthor(i18n("Tuomas Suutari"), QString(), QStringLiteral("thsuut@utu.fi"));
    aboutData.addAuthor(i18n("Miika Turkia"), QString(), QStringLiteral("miika.turkia@gmail.com"));
    aboutData.addAuthor(i18n("Henner Zeller"), QString(), QStringLiteral("h.zeller@acm.org"));

    // initialize the commandline parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption infoOption { QString::fromUtf8("info"), i18nc("@info:shell", "Print information about thumbnail cache.") };
    parser.addOption(infoOption);
    QCommandLineOption convertV5ToV4Option { QString::fromUtf8("convertV5ToV4"), i18nc("@info:shell", "Convert thumbnailindex to format suitable for KPhotoAlbum >= 4.3.") };
    parser.addOption(convertV5ToV4Option);
    QCommandLineOption verifyOption { QString::fromUtf8("verify"), i18nc("@info:shell", "Verify thumbnail cache consistency.") };
    parser.addOption(verifyOption);
    parser.addPositionalArgument(QString::fromUtf8("imageDir"), i18nc("@info:shell", "The directory containing the .thumbnail directory."));

    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);
    QTextStream console { stdout };

    const auto args = parser.positionalArguments();
    if (args.empty()) {
        qWarning("Missing argument!");
        return 1;
    }
    const auto imageDir = QDir { args.first() };
    if (!imageDir.exists()) {
        qWarning("Not a directory!");
        return 1;
    }
    if (parser.isSet(convertV5ToV4Option)) {
        const QString indexFile = imageDir.absoluteFilePath(QString::fromUtf8(".thumbnails/thumbnailindex"));
        return convertV5ToV4Cache(indexFile);
    }

    int returnValue = 0;
    DB::DummyUIDelegate uiDelegate;
    Settings::SettingsData::setup(imageDir.path(), uiDelegate);
    const auto thumbnailDir = imageDir.absoluteFilePath(ImageManager::defaultThumbnailDirectory());
    const ImageManager::ThumbnailCache cache { thumbnailDir };
    if (parser.isSet(infoOption)) {
        console << i18nc("@info:shell", "Thumbnail cache directory: %1\n", thumbnailDir);
        console << i18nc("@info:shell", "Thumbnailindex file version: %1\n", cache.actualFileVersion());
        console << i18nc("@info:shell", "Maximum supported thumbnailindex file version: %1\n", cache.preferredFileVersion());
        console << i18nc("@info:shell", "Thumbnail storage size: %1\n", cache.thumbnailSize());
        if (cache.actualFileVersion() < 5) {
            console << i18nc("@info:shell", "Note: Thumbnail storage size is defined in the configuration file prior to v5.\n");
        }
    }
    if (parser.isSet(verifyOption)) {
        const auto incorrectDimensions = cache.findIncorrectlySizedThumbnails();
        if (incorrectDimensions.isEmpty()) {
            console << i18nc("@info:shell", "No inconsistencies found.\n");
        } else {
            returnValue = 1;
            console << i18nc("@info:shell This line is printed before a list of file names.", "The following thumbnails appear to have incorrect sizes:\n");
            for (const auto &filename : incorrectDimensions) {
                console << filename.absolute() << "\n";
            }
        }
    }

    return returnValue;
    // so far, we don't need an event loop:
    //// immediately quit the event loop:
    //QTimer::singleShot(0, &app, &QCoreApplication::quit);
    //return QCoreApplication::exec();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
