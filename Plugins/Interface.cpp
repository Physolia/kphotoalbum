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

#include <config-kpa-kipi.h>

#ifdef HASKIPI
#include "Plugins/Interface.h"
#include <libkipi/imagecollection.h>
#include "Plugins/ImageCollection.h"
#include "Plugins/ImageInfo.h"
#include "DB/ImageDB.h"
#include "MainWindow/Window.h"
#include "Plugins/CategoryImageCollection.h"
#include <klocale.h>
#include "DB/ImageInfo.h"
#include "Browser/BrowserWidget.h"
#include "Plugins/ImageCollectionSelector.h"
//Added by qt3to4:
#include <Q3ValueList>

Plugins::Interface::Interface( QObject *parent, const char *name )
    :KIPI::Interface( parent, name )
{
    connect( Browser::BrowserWidget::instance(), SIGNAL( pathChanged( const BreadcrumbList& ) ), this, SLOT( pathChanged( const BreadcrumbList& ) ) );
}

KIPI::ImageCollection Plugins::Interface::currentAlbum()
{
    return KIPI::ImageCollection( new Plugins::ImageCollection( Plugins::ImageCollection::CurrentAlbum ) );
}

KIPI::ImageCollection Plugins::Interface::currentSelection()
{
    if (!MainWindow::Window::theMainWindow()->selected().isEmpty())
        return KIPI::ImageCollection( new Plugins::ImageCollection( Plugins::ImageCollection::CurrentSelection ) );
    else
        return KIPI::ImageCollection(0);
}

QList<KIPI::ImageCollection> Plugins::Interface::allAlbums()
{
    QList<KIPI::ImageCollection> result;
    DB::ImageSearchInfo context = MainWindow::Window::theMainWindow()->currentContext();
    QString category = MainWindow::Window::theMainWindow()->currentBrowseCategory();
    if ( category.isNull() )
        category = Settings::SettingsData::instance()->albumCategory();

    QMap<QString,uint> categories = DB::ImageDB::instance()->classify( context, category, DB::Image );

    for( QMap<QString,uint>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        CategoryImageCollection* col = new CategoryImageCollection( context, category, it.key() );
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo Plugins::Interface::info( const KUrl& url )
{
    return KIPI::ImageInfo(new Plugins::ImageInfo(this, url));
}

void Plugins::Interface::refreshImages( const KUrl::List& urls )
{
    emit imagesChanged( urls );
}

int Plugins::Interface::features() const
{
    return
        KIPI::ImagesHasComments |
        KIPI::ImagesHasTime |
        KIPI::HostSupportsDateRanges |
        KIPI::HostAcceptNewImages |
        KIPI::ImagesHasTitlesWritable;
}

bool Plugins::Interface::addImage( const KUrl& url, QString& errmsg )
{
    QString dir = url.path();
    QString root = Settings::SettingsData::instance()->imageDirectory();
    if ( !dir.startsWith( root ) ) {
        errmsg = i18n("<p>Image needs to be placed in a sub directory of your photo album, "
                      "which is rooted at %1. Image path was %2</p>",root , dir );
        return false;
    }

    dir = dir.mid( root.length() );
    DB::ImageInfoPtr info( new DB::ImageInfo( dir ) );
    DB::ImageInfoList list;
    list.append( info );
    DB::ImageDB::instance()->addImages( list );
    return true;
}

void Plugins::Interface::delImage( const KUrl& url )
{
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( url.path(), DB::AbsolutePath );
    if ( info ) {
        DB::Result list;
        list.append(DB::ImageDB::instance()->ID_FOR_FILE(info->fileName(DB::AbsolutePath)));
        DB::ImageDB::instance()->deleteList( list );
    }
}

void Plugins::Interface::slotSelectionChanged( bool b )
{
    emit selectionChanged( b );
}

void Plugins::Interface::pathChanged( const Browser::BreadcrumpList& path )
{
    static Browser::BreadcrumbList _path;
    if ( _path != path ) {
        emit currentAlbumChanged( true );
        _path = path;
    }
}

KIPI::ImageCollectionSelector* Plugins::Interface::imageCollectionSelector(QWidget *parent)
{
    return new ImageCollectionSelector( parent, this );
}

KIPI::UploadWidget* Plugins::Interface::uploadWidget(QWidget *)
{
    qDebug("Interface::uploadWidget(...) not yet implemented!");
   //TODO
    return 0;
}

#include "Interface.moc"
#endif // KIPI
