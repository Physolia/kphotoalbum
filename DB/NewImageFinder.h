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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef NEWIMAGEFINDER_H
#define NEWIMAGEFINDER_H
#include <q3dict.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>
#include "DB/MD5Map.h"
#include "ImageInfo.h"

namespace DB
{

class NewImageFinder
{
public:
    bool findImages();
    bool calculateMD5sums( const QStringList& list, DB::MD5Map* map, bool* wasCanceled = 0 );

protected:
    void searchForNewFiles( const Q3Dict<void>& loadedFiles, QString directory );
    void loadExtraFiles();
    ImageInfoPtr loadExtraFile( const QString& name, DB::MediaType type );
    MD5 MD5Sum( const QString& fileName );

private:
    typedef Q3ValueList< QPair< QString, DB::MediaType > > LoadList;
    LoadList _pendingLoad;
};
}

#endif /* NEWIMAGEFINDER_H */

