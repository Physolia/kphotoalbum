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

#ifndef SEARCHFOLDER_H
#define SEARCHFOLDER_H
#include "Folder.h"
#include <QPixmap>

namespace AnnotationDialog {class Dialog;}

namespace Browser
{

class SearchFolder :public Folder {

public:
    SearchFolder( const DB::ImageSearchInfo& info, BrowserWidget* browser );
    OVERRIDE FolderAction* action( bool ctrlDown = false );
    OVERRIDE QPixmap pixmap();
    OVERRIDE QString text() const;
    OVERRIDE QString imagesLabel() const;
    OVERRIDE QString videosLabel() const;
    AnnotationDialog::Dialog* _config;
};

}

#endif /* SEARCHFOLDER_H */
