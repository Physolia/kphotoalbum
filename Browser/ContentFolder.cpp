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

#include "ContentFolder.h"
#include "Settings/SettingsData.h"
#include "TypeFolder.h"
#include "ImageFolder.h"
#include <klocale.h>
#include "DB/ImageDB.h"
#include "SearchFolder.h"
#include <kglobal.h>
#include <kiconloader.h>
#include "DB/MemberMap.h"
#include "qregexp.h"
//Added by qt3to4:
#include <QPixmap>
#include "ContentFolderAction.h"
#include <KIcon>

Browser::ContentFolder::ContentFolder( const DB::CategoryPtr& category, const QString& value, DB::MediaCount count,
                                       const DB::ImageSearchInfo& info, BrowserWidget* parent )
    :Folder( info, parent ), _category( category ), _value( value )
{
    _info.addAnd( _category->name(), _value );
    setCount( count );
}

QPixmap Browser::ContentFolder::pixmap()
{
    if ( _category->viewType() == DB::Category::ListView || _category->viewType() == DB::Category::IconView ) {
        if ( DB::ImageDB::instance()->memberMap().isGroup( _category->name(), _value ) )
            return KIcon( QString::fromLatin1( "folder_image" ) ).pixmap(22);
        else {
            return _category->icon();
        }
    }
    else
        return Settings::SettingsData::instance()->categoryImage( _category->name(), _value, _category->thumbnailSize() );
}

QString Browser::ContentFolder::text() const
{
    if ( _value == DB::ImageDB::NONE() ) {
        if ( _info.option(_category->name()) == DB::ImageDB::NONE() )
            return i18n( "None" );
        else
            return i18n( "No other" );
    }
    else if ( _value == QString::fromLatin1( "Video" ) )
        return i18n("Video");
    else if ( _value == QString::fromLatin1( "Image" ) )
        return i18n("Image");

    else {
      if ( _category->name() == QString::fromLatin1( "Folder" ) ) {
        QRegExp rx( QString::fromLatin1( "(.*/)(.*)$") );
        QString value = _value;
        value.replace( rx, QString::fromLatin1("\\2") );
        return value;
      } else {
        return _value;
      }
    }
}



Browser::FolderAction* Browser::ContentFolder::action( bool ctrlDown )
{
    DB::MediaCount counts = DB::ImageDB::instance()->count( _info );
    bool loadImages = (  counts.total() < static_cast<uint>(Settings::SettingsData::instance()->autoShowThumbnailView()));
    if ( ctrlDown ) loadImages = !loadImages;

    if ( loadImages ) {
        DB::ImageSearchInfo info = _info;
        return new ImageFolderAction( info, _browser );
    }

    return new ContentFolderAction( _info, _browser );
}


int Browser::ContentFolder::compare( Folder* other, int col, bool asc ) const
{
    if ( col == 0 ) {
        if ( _value == DB::ImageDB::NONE() )
            return ( asc ? -1 : 1);
        ContentFolder* o = static_cast<ContentFolder*>( other );
        if ( o->_value == DB::ImageDB::NONE() )
            return ( asc ? 1: -1 );
    }

    return Folder::compare( other, col, asc );
}

QString Browser::ContentFolder::imagesLabel() const
{
    return i18np("1 Image", "%1 Images", _count.images());
}

QString Browser::ContentFolder::videosLabel() const
{
    return i18np("1 Movie", "%1 Movies", _count.videos());
}