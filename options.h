/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qobject.h>
#include <qdom.h>
#include "membermap.h"
#include "imagesearchinfo.h"
#include "category.h"
class ImageConfig;


#define property__( type, group, prop, setFunction, defaultValue ) \
    void setFunction( type val )                                     \
    {                                                             \
        setValue( #group, #prop, val );                     \
    }                                             \
    type prop() const                          \
    {                                             \
        return value( #group, #prop, defaultValue ); \
    }

#define intProperty( group, prop, setFunction, defaultValue ) property__( int, group, prop, setFunction, defaultValue )
#define boolProperty( group, prop, setFunction, defaultValue ) property__( bool, group, prop, setFunction, defaultValue )
#define colorProperty( group, prop, setFunction, defaultValue ) property__( QColor, group, prop, setFunction, defaultValue )
#define sizeProperty(  group, prop, setFunction, defaultValue ) property__( QSize, group, prop, setFunction, defaultValue )

class Options :public QObject {
    Q_OBJECT

public:
    static Options* instance();
    static bool ready();
    static void setup( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory );
    // -------------------------------------------------- General
    boolProperty( General, useEXIFRotate, setUseEXIFRotate, true );
    boolProperty( General, useEXIFComments, setUseEXIFComments, true );
    boolProperty( General, searchForImagesOnStartup, setSearchForImagesOnStartup, true );
    intProperty( General, autoSave, setAutoSave, 5 );

    QSize histogramSize() const;
    void setHistogramSize( const QSize& size );

    // -------------------------------------------------- Thumbnails
    intProperty( Thumbnails, thumbSize, setThumbSize, 128 );
    intProperty( Thumbnails, previewSize, setPreviewSize, 256 );
    intProperty( Thumbnails, rowSpacing, setRowSpacing, 10 );
    boolProperty( Thumbnails, displayLabels, setDisplayLabels, true );
    intProperty( Thumbnails, autoShowThumbnailView, setAutoShowThumbnailView, 0 );
    boolProperty( Thumbnails, alignColumns, setAlignColumns, true );
    colorProperty( Thumbnails, thumbNailBackgroundColor, setThumbNailBackgroundColor, Qt::black );

    // -------------------------------------------------- Viewer
    sizeProperty( Viewer, viewerSize, setViewerSize, QSize( 800,600 ) );
    sizeProperty( Viewer, slideShowSize, setSlideShowSize, QSize( 800, 600 ) );
    boolProperty( Viewer, launchViewerFullScreen, setLaunchViewerFullScreen, false );
    boolProperty( Viewer, launchSlideShowFullScreen, setLaunchSlideShowFullScreen, false );
    intProperty( Viewer, slideShowInterval, setSlideShowInterval, 5 );
    intProperty( Viewer, viewerCacheSize, setViewerCacheSize, 25 );

    boolProperty( Viewer, showInfoBox, setShowInfoBox, true );
    boolProperty( Viewer, showDrawings, setShowDrawings, true );
    boolProperty( Viewer, showDescription, setShowDescription, true );
    boolProperty( Viewer, showDate, setShowDate, true );
    boolProperty( Viewer, showTime, setShowTime, true );


    enum ViewSortType { SortLastUse, SortAlpha };
    void setViewSortType( ViewSortType );
    ViewSortType viewSortType() const;

    enum Position { Bottom = 0, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    bool showOption( const QString& ) const;
    void setShowOption( const QString& category, bool b );

    Position infoBoxPosition() const;
    void setInfoBoxPosition( Position pos );


    // -------------------------------------------------- Miscellaneous
    void setFromDate( const QDate& );
    QDate fromDate() const;
    void setToDate( const QDate& );
    QDate toDate() const;

    // -------------------------------------------------- Options
    void setOption( const QString& category,  const QStringList& value );
    void addOption( const QString& category,  const QString& value );
    void removeOption( const QString& category, const QString& value );
    QStringList optionValue( const QString& category ) const;
    QStringList optionValueInclGroups( const QString& category ) const;
    void renameOption( const QString& category, const QString& oldValue, const QString& newValue );

    QString fileForCategoryImage(  const QString& category, QString member ) const;
    void setOptionImage( const QString& category, QString, const QImage& image );
    QImage optionImage( const QString& category,  QString, int size ) const;

    // -------------------------------------------------- Categories
    QString albumCategory() const;
    void setAlbumCategory(  const QString& category );

    // -------------------------------------------------- Member Groups
    const MemberMap& memberMap();
    void setMemberMap( const MemberMap& );

    // -------------------------------------------------- misc
    enum TimeStampTrust {
        Always = 0,
        Ask = 1,
        Never = 2
    };

    bool trustTimeStamps();
    void setTTimeStamps( TimeStampTrust );
    TimeStampTrust tTimeStamps() const;

    void save( QDomElement top );

    QString imageDirectory() const;

    QString HTMLBaseDir() const;
    void setHTMLBaseDir( const QString& dir );

    QString HTMLBaseURL() const;
    void setHTMLBaseURL( const QString& dir );

    QString HTMLDestURL() const;
    void setHTMLDestURL( const QString& dir );

    void setCurrentLock( const ImageSearchInfo&, bool exclude );
    ImageSearchInfo currentLock() const;

    void setLocked( bool );
    bool isLocked() const;
    bool lockExcludes() const;

    void setPassword( const QString& passwd );
    QString password() const;

    enum WindowType { MainWindow = 0, ConfigWindow = 1 };
    void setWindowGeometry( WindowType, const QRect& geometry );
    QRect windowGeometry( WindowType ) const;

protected:
    void createSpecialCategories();

    int value( const char* group, const char* option, int defaultValue ) const;
    QString value( const char* group, const char* option, const char* defaultValue ) const;
    bool value( const char* group, const char* option, bool defaultValue ) const;
    QColor value( const char* group, const char* option, const QColor& defaultValue ) const;
    QSize value( const char* group, const char* option, const QSize& defaultValue ) const;

    void setValue( const char* group, const char* option, int value );
    void setValue( const char* group, const char* option, const QString& value );
    void setValue( const char* group, const char* option, bool value );
    void setValue( const char* group, const char* option, const QColor& value );
    void setValue( const char* group, const char* option, const QSize& value );
    const char* windowTypeToString( WindowType tp ) const;

signals:
    void changed();
    void renamedOption( const QString& category, const QString& oldName, const QString& newName );
    void deletedOption( const QString& category, const QString& name );
    void locked( bool lock, bool exclude );
    void viewSortTypeChanged( Options::ViewSortType );
    void histogramSizeChanged( const QSize& );

private:
    Options( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory  );
    static Options* _instance;

    bool _trustTimeStamps, _markNew, _hasAskedAboutTimeStamps;

    friend class CategoryCollection;
    QMap<QString, QStringList> _options;
    QString _imageDirectory, _htmlBaseDir, _htmlBaseURL, _htmlDestURL;

    MemberMap _members;
    ImageSearchInfo _currentLock;
    bool _locked, _exclude;
    QString _passwd;
};

#undef intProperty
#undef boolProperty
#undef property__

#endif /* OPTIONS_H */

