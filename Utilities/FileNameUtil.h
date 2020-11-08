/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILENAME_UTIL_H
#define FILENAME_UTIL_H

#include <QString>

namespace Utilities
{
QString stripEndingForwardSlash(const QString &fileName);

QString absoluteImageFileName(const QString &relativeName);
QString imageFileNameToAbsolute(const QString &fileName);

QString relativeFolderName(const QString &fileName);
}

#endif /* FILENAME_UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
