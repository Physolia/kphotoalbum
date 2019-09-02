/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

// Local includes
#include "DescriptionUtil.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "Exif/Info.h"
#include "Logging.h"
#include "Settings/SettingsData.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QList>
#include <QTextCodec>
#include <QUrl>

namespace
{
const QLatin1String LINE_BREAK("<br/>");
}

/**
 * Add a line label + info text to the result text if info is not empty.
 * If the result already contains something, a HTML newline is added first.
 * To be used in createInfoText().
 */
static void AddNonEmptyInfo(const QString &label, const QString &infoText, QString *result)
{
    if (infoText.isEmpty()) {
        return;
    }
    if (!result->isEmpty()) {
        *result += LINE_BREAK;
    }
    result->append(label).append(infoText);
}

/**
 * Given an ImageInfoPtr this function will create an HTML blob about the
 * image. The blob is used in the viewer and in the tool tip box from the
 * thumbnail view.
 *
 * As the HTML text is created, the parameter linkMap is filled with
 * information about hyperlinks. The map maps from an index to a pair of
 * (categoryName, categoryItem). This linkMap is used when the user selects
 * one of the hyberlinks.
 */
QString Utilities::createInfoText(DB::ImageInfoPtr info, QMap<int, QPair<QString, QString>> *linkMap)
{
    Q_ASSERT(info);

    QString result;
    if (Settings::SettingsData::instance()->showFilename()) {
        AddNonEmptyInfo(i18n("<b>File Name: </b> "), info->fileName().relative(), &result);
    }

    if (Settings::SettingsData::instance()->showDate()) {
        QString dateString = info->date().toString(Settings::SettingsData::instance()->showTime() ? true : false);
        if (!dateString.isEmpty()) {
            dateString.append(i18n(" (%1)", timeAgo(info)));
        }
        AddNonEmptyInfo(i18n("<b>Date: </b> "), dateString, &result);
    }

    /* XXX */
    if (Settings::SettingsData::instance()->showImageSize() && info->mediaType() == DB::Image) {
        const QSize imageSize = info->size();
        // Do not add -1 x -1 text
        if (imageSize.width() >= 0 && imageSize.height() >= 0) {
            const double megapix = imageSize.width() * imageSize.height() / 1000000.0;
            QString infoText = i18nc("width x height", "%1x%2", QString::number(imageSize.width()), QString::number(imageSize.height()));
            if (megapix > 0.05) {
                infoText += i18nc("short for: x megapixels", " (%1MP)", QString::number(megapix, 'f', 1));
            }
            const double aspect = (double)imageSize.width() / (double)imageSize.height();
            // 0.995 - 1.005 can still be considered quadratic
            if (aspect > 1.005)
                infoText += i18nc("aspect ratio", " (%1:1)", QLocale::system().toString(aspect, 'f', 2));
            else if (aspect >= 0.995)
                infoText += i18nc("aspect ratio", " (1:1)");
            else
                infoText += i18nc("aspect ratio", " (1:%1)", QLocale::system().toString(1.0 / aspect, 'f', 2));
            AddNonEmptyInfo(i18n("<b>Image Size: </b> "), infoText, &result);
        }
    }

    if (Settings::SettingsData::instance()->showRating()) {
        if (info->rating() != -1) {
            if (!result.isEmpty())
                result += QString::fromLatin1("<br/>");
            QUrl rating;
            rating.setScheme(QString::fromLatin1("kratingwidget"));
            // we don't use the host part, but if we don't set it, we can't use port:
            rating.setHost(QString::fromLatin1("int"));
            rating.setPort(qMin(qMax(static_cast<short int>(0), info->rating()), static_cast<short int>(10)));
            result += QString::fromLatin1("<img src=\"%1\"/>").arg(rating.toString(QUrl::None));
        }
    }

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    int link = 0;
    Q_FOREACH (const DB::CategoryPtr category, categories) {
        const QString categoryName = category->name();
        if (category->doShow()) {
            StringSet items = info->itemsOfCategory(categoryName);

            if (Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured()
                && !Settings::SettingsData::instance()->untaggedImagesTagVisible()) {

                if (categoryName == Settings::SettingsData::instance()->untaggedCategory()) {
                    if (items.contains(Settings::SettingsData::instance()->untaggedTag())) {
                        items.remove(Settings::SettingsData::instance()->untaggedTag());
                    }
                }
            }

            if (!items.empty()) {
                QString title = QString::fromUtf8("<b>%1: </b> ").arg(category->name());
                QString infoText;
                bool first = true;
                Q_FOREACH (const QString &item, items) {
                    if (first)
                        first = false;
                    else
                        infoText += QString::fromLatin1(", ");

                    if (linkMap) {
                        ++link;
                        (*linkMap)[link] = QPair<QString, QString>(categoryName, item);
                        infoText += QString::fromLatin1("<a href=\"%1\">%2</a>").arg(link).arg(item);
                        infoText += formatAge(category, item, info);
                    } else
                        infoText += item;
                }
                AddNonEmptyInfo(title, infoText, &result);
            }
        }
    }

    if (Settings::SettingsData::instance()->showLabel()) {
        AddNonEmptyInfo(i18n("<b>Label: </b> "), info->label(), &result);
    }

    if (Settings::SettingsData::instance()->showDescription() && !info->description().trimmed().isEmpty()) {
        AddNonEmptyInfo(i18n("<b>Description: </b> "), info->description(),
                        &result);
    }

    QString exifText;
    if (Settings::SettingsData::instance()->showEXIF()) {
        typedef QMap<QString, QStringList> ExifMap;
        typedef ExifMap::const_iterator ExifMapIterator;
        ExifMap exifMap = Exif::Info::instance()->infoForViewer(info->fileName(), Settings::SettingsData::instance()->iptcCharset());

        for (ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt) {
            if (exifIt.key().startsWith(QString::fromLatin1("Exif.")))
                for (QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt) {
                    QString exifName = exifIt.key().split(QChar::fromLatin1('.')).last();
                    AddNonEmptyInfo(QString::fromLatin1("<b>%1: </b> ").arg(exifName),
                                    *valuesIt, &exifText);
                }
        }

        QString iptcText;
        for (ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt) {
            if (!exifIt.key().startsWith(QString::fromLatin1("Exif.")))
                for (QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt) {
                    QString iptcName = exifIt.key().split(QChar::fromLatin1('.')).last();
                    AddNonEmptyInfo(QString::fromLatin1("<b>%1: </b> ").arg(iptcName),
                                    *valuesIt, &iptcText);
                }
        }

        if (!iptcText.isEmpty()) {
            if (exifText.isEmpty())
                exifText = iptcText;
            else
                exifText += QString::fromLatin1("<hr>") + iptcText;
        }
    }

    if (!result.isEmpty() && !exifText.isEmpty())
        result += QString::fromLatin1("<hr>");
    result += exifText;

    return result;
}

using DateSpec = QPair<int, char>;
DateSpec dateDiff(const QDate &birthDate, const QDate &imageDate)
{
    const int bday = birthDate.day();
    const int iday = imageDate.day();
    const int bmonth = birthDate.month();
    const int imonth = imageDate.month();
    const int byear = birthDate.year();
    const int iyear = imageDate.year();

    // Image before birth
    const int diff = birthDate.daysTo(imageDate);
    if (diff < 0)
        return qMakePair(0, 'I');

    if (diff < 31)
        return qMakePair(diff, 'D');

    int months = (iyear - byear) * 12;
    months += (imonth - bmonth);
    months += (iday >= bday) ? 0 : -1;

    if (months < 24)
        return qMakePair(months, 'M');
    else
        return qMakePair(months / 12, 'Y');
}

QString formatDate(const DateSpec &date)
{
    if (date.second == 'I')
        return {};
    else if (date.second == 'D')
        return i18np("1 day", "%1 days", date.first);
    else if (date.second == 'M')
        return i18np("1 month", "%1 months", date.first);
    else
        return i18np("1 year", "%1 years", date.first);
}

void test()
{
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1971, 7, 11))) == QString::fromLatin1("0 days"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1971, 8, 10))) == QString::fromLatin1("30 days"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1971, 8, 11))) == QString::fromLatin1("1 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1971, 8, 12))) == QString::fromLatin1("1 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1971, 9, 10))) == QString::fromLatin1("1 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1971, 9, 11))) == QString::fromLatin1("2 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1972, 6, 10))) == QString::fromLatin1("10 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1972, 6, 11))) == QString::fromLatin1("11 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1972, 6, 12))) == QString::fromLatin1("11 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1972, 7, 10))) == QString::fromLatin1("11 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1972, 7, 11))) == QString::fromLatin1("12 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1972, 7, 12))) == QString::fromLatin1("12 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1972, 12, 11))) == QString::fromLatin1("17 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971, 7, 11), QDate(1973, 7, 11))) == QString::fromLatin1("2 years"));
}

QString Utilities::formatAge(DB::CategoryPtr category, const QString &item, DB::ImageInfoPtr info)
{
    // test(); // I wish I could get my act together to set up a test suite.
    const QDate birthDate = category->birthDate(item);
    const QDate start = info->date().start().date();
    const QDate end = info->date().end().date();

    if (birthDate.isNull() || start.isNull())
        return {};

    if (start == end)
        return QString::fromUtf8(" (%1)").arg(formatDate(dateDiff(birthDate, start)));
    else {
        DateSpec lower = dateDiff(birthDate, start);
        DateSpec upper = dateDiff(birthDate, end);
        if (lower == upper)
            return QString::fromUtf8(" (%1)").arg(formatDate(lower));
        else if (lower.second == 'I')
            return QString::fromUtf8(" (&lt; %1)").arg(formatDate(upper));
        else {
            if (lower.second == upper.second)
                return QString::fromUtf8(" (%1-%2)").arg(lower.first).arg(formatDate(upper));
            else
                return QString::fromUtf8(" (%1-%2)").arg(formatDate(lower)).arg(formatDate(upper));
        }
    }
}

QString Utilities::timeAgo(const DB::ImageInfoPtr info)
{
    const QDate startDate = info->date().start().date();
    const QDate endDate = info->date().end().date();
    if (startDate == endDate) {
        return i18n("%1 ago", timeAgo(startDate));
    } else {
        const QString startTimeAgo = timeAgo(startDate);
        const QString endTimeAgo = timeAgo(endDate);
        if (startTimeAgo == endTimeAgo) {
            return i18n("%1 ago", startTimeAgo);
        } else {
            return i18n("%1 to %2 ago", startTimeAgo, endTimeAgo);
        }
    }
}

QString Utilities::timeAgo(const QDate &date)
{
    const QDate today = QDate::currentDate();
    const qint64 daysPassed = today.toJulianDay() - date.toJulianDay();

    if (daysPassed <= 0) {
        // The photo has been taken today or has a date in the future
        return QString();
    }

    if (daysPassed < 7) {
        // Less than a week --> display the days
        return i18np("1 day", "%1 days", daysPassed);
    }

    if (daysPassed < 30) {
        // Less than a month --> display the (approximate) weeks
        return i18np("1 week", "%1 weeks", qRound64((double)daysPassed / 7.0));
    }

    if (daysPassed < 365) {
        // Less than a year --> display the (approximate) months
        // We take 30.44 days per month, as this is the result of 365.25 / 12
        const int months = qRound64((double)daysPassed / 30.44);
        if (months == 12) {
            return i18np("1 year", "%1 years", 1);
        } else {
            return i18np("1 month", "%1 months", months);
        }
    }

    if (daysPassed < 1826) {
        // Less than five years --> display years and months
        const int years = today.year() - date.year();
        const int months = today.month() - date.month();
        if (months == 0) {
            return i18np("1 year", "%1 years", years);
        }
        if (months > 0) {
            return i18n("%1 and %2", i18np("1 year", "%1 years", years), i18np("1 month", "%1 months", months));
        }
        return i18n("%1 and %2", i18np("1 year", "%1 years", years - 1), i18np("1 month", "%1 months", 12 + months));
    }

    // More than five years --> display only the years
    return i18np("1 year", "%1 years", today.year() - date.year());
}
