/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CACHEFILEINFO_H
#define CACHEFILEINFO_H

namespace ImageManager
{
struct CacheFileInfo {
    CacheFileInfo() {}
    CacheFileInfo(int fileIndex, int offset, int size)
        : fileIndex(fileIndex)
        , offset(offset)
        , size(size)
    {
    }

    int fileIndex;
    int offset;
    int size;
};
}

#endif /* CACHEFILEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
