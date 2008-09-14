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

#include "ThumbnailToolTip.h"

#include <Q3Frame>
#include <QDesktopWidget>
#include <QEvent>
#include <QLabel>
#include <QTemporaryFile>
#include <QApplication>
#include <QCursor>
#include <QToolTip>
#include <qmime.h>

#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "ImageManager/Manager.h"
#include "Settings/SettingsData.h"
#include "ThumbnailWidget.h"
#include "Utilities/Util.h"

QTemporaryFile* _tmpFileForThumbnailView = 0;

/**
   \class ThumbnailToolTip
   This class takes care of showing tooltips for the individual items in the thumbnail view.
   I tried implementing this with QToolTip::maybeTip() on the iconview, but it had the
   disadvantages that either the tooltip would not follow the
   mouse( and would therefore stand on top of the image), or it flickered.
*/

ThumbnailView::ThumbnailToolTip::ThumbnailToolTip( ThumbnailWidget* view )
    : QLabel( view, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WType_TopLevel
              | Qt::WX11BypassWM
              | Qt::WStyle_Tool ), _view( view ),
      _widthInverse( false ), _heightInverse( false )
{
    setAlignment( Qt::AlignLeft | Qt::AlignTop );
    setFrameStyle( Q3Frame::Box | Q3Frame::Plain );
    setLineWidth(1);
    setMargin(1);
#ifdef KDAB_TEMPORARILY_REMOVED
    setPalette( QToolTip::palette() );
#endif //KDAB_TEMPORARILY_REMOVED

    setWindowOpacity(0.5);
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(QPalette::Background, QColor(0,0,0,170)); // r,g,b,A
    p.setColor(QPalette::WindowText, Qt::white );
    setPalette(p);
}

bool ThumbnailView::ThumbnailToolTip::eventFilter( QObject* o , QEvent* event )
{
    if ( o == _view->viewport() && event->type() == QEvent::Leave )
        hide();

    else if ( event->type() == QEvent::MouseMove )
        showToolTips( false );
    return false;
}

void ThumbnailView::ThumbnailToolTip::showToolTips( bool force )
{
    DB::ResultId id = _view->mediaIdUnderCursor();
    if ( id.isNull() )
        return;
    
    QString fileName = id.fetchInfo()->fileName(DB::AbsolutePath);
    if ( force || (fileName != _currentFileName) ) {
        if ( loadImage( fileName ) ) {
            setText( QString::null );
            int size = Settings::SettingsData::instance()->previewSize();
            if ( size != 0 ) {
                setText( QString::fromLatin1("<table cols=\"2\" cellpadding=\"10\"><tr><td><img src=\"%1\"></td><td>%2</td></tr>")
                         .arg(_tmpFileForThumbnailView->fileName()).
                         arg(Utilities::createInfoText( DB::ImageDB::instance()->info( fileName, DB::AbsolutePath ), 0 ) ) );
            }
            else {
                setText( QString::fromLatin1("<p>%1</p>").arg( Utilities::createInfoText( DB::ImageDB::instance()->info( fileName, DB::AbsolutePath ), 0 ) ) );
            }
            setWordWrap( true );
        }

        _currentFileName = fileName;
        resize( sizeHint() );
        _view->setFocus();
    }

    placeWindow();
    show();
}


void ThumbnailView::ThumbnailToolTip::setActive( bool b )
{
    if ( b ) {
        showToolTips(true);
        _view->viewport()->installEventFilter( this );
        show();
    }
    else {
        _view->viewport()->removeEventFilter( this );
        hide();
    }
}

void ThumbnailView::ThumbnailToolTip::placeWindow()
{
// First try to set the position.
    QPoint pos = QCursor::pos() + QPoint( 20, 20 );
    if ( _widthInverse )
        pos.setX( pos.x() - 30 - width() );
    if ( _heightInverse )
        pos.setY( pos.y() - 30 - height() );

    QRect geom = qApp->desktop()->screenGeometry( QCursor::pos() );

    // Now test whether the window moved outside the screen
    if ( _widthInverse ) {
        if ( pos.x() <  geom.x() ) {
            pos.setX( QCursor::pos().x() + 20 );
            _widthInverse = false;
        }
    }
    else {
        if ( pos.x() + width() > geom.right() ) {
            pos.setX( QCursor::pos().x() - width() );
            _widthInverse = true;
        }
    }

    if ( _heightInverse ) {
        if ( pos.y() < geom.y()  ) {
            pos.setY( QCursor::pos().y() + 10 );
            _heightInverse = false;
        }
    }
    else {
        if ( pos.y() + height() > geom.bottom() ) {
            pos.setY( QCursor::pos().y() - 10 - height() );
            _heightInverse = true;
        }
    }

    move( pos );
}


bool ThumbnailView::ThumbnailToolTip::loadImage( const QString& fileName )
{
    int size = Settings::SettingsData::instance()->previewSize();
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( fileName, DB::AbsolutePath );
    if ( size != 0 ) {
        if ( fileName != _currentFileName ) {
            ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( size, size ), info->angle(), this );
            // request->setCache();  // TODO: do caching in callback.
            request->setPriority( ImageManager::Viewer );
            ImageManager::Manager::instance()->load( request );
            return false;
        }
    }
    return true;
}

void ThumbnailView::ThumbnailToolTip::pixmapLoaded( const QString& fileName, const QSize& /*size*/,
                                                    const QSize& /*fullSize*/, int /*angle*/, const QImage& image, const bool /*loadedOK*/)
{
    delete _tmpFileForThumbnailView;
    _tmpFileForThumbnailView = new QTemporaryFile(this);
    _tmpFileForThumbnailView->open();

    image.save(_tmpFileForThumbnailView, "PNG" );
    if ( fileName == _currentFileName )
        showToolTips(true);
}

#include "ThumbnailToolTip.moc"
