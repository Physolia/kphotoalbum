
/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "export.h"
#include <kfiledialog.h>
#include <kzip.h>
#include <qfileinfo.h>
#include <time.h>
#include "util.h"
#include <qprogressdialog.h>
#include <qeventloop.h>
#include <klocale.h>
#include "imagemanager.h"
#include <qapplication.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <kimageio.h>

void Export::imageExport( const ImageInfoList& list )
{
    ExportConfig config;
    if ( config.exec() == QDialog::Rejected )
        return;

    int maxSize = -1;
    if ( config._enforeMaxSize->isChecked() )
        maxSize = config._maxSize->value();

    // Ask for zip file name
    QString zipFile = KFileDialog::getSaveFileName( QString::null, QString::fromLatin1( "*.kim|KimDaBa export files" ), 0 );
    if ( zipFile.isNull() )
        return;

    bool ok;
    Export* exp = new Export( list, zipFile, config._compress->isChecked(), maxSize, config.imageFileLocation(),
                              QString::fromLatin1( "" ), ok );
    delete exp; // It will not return before done - we still need a class to connect slots etc.

    if ( ok )
        showUsageDialog();
}

// PENDING(blackie) add warning if images are to be copied into a non empty directory.
ExportConfig::ExportConfig()
    : KDialogBase( KDialogBase::Plain, i18n("Export Configuration"), KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
                   KDialogBase::Ok, 0, "export config" )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    // Compress
    _compress = new QCheckBox( i18n("Compress Export File"), top );
    lay1->addWidget( _compress );

    // Enforece max size
    QHBoxLayout* hlay = new QHBoxLayout( lay1, 6 );
    _enforeMaxSize = new QCheckBox( i18n( "Limit maximum dimension of images to: " ), top, "_enforeMaxSize" );
    hlay->addWidget( _enforeMaxSize );

    _maxSize = new QSpinBox( 100, 4000, 50, top, "_maxSize" );
    hlay->addWidget( _maxSize );
    _maxSize->setValue( 800 );

    connect( _enforeMaxSize, SIGNAL( toggled( bool ) ), _maxSize, SLOT( setEnabled( bool ) ) );
    _maxSize->setEnabled( false );

    // Include images
    QVButtonGroup* grp = new QVButtonGroup( i18n("How to handle images"), top );
    lay1->addWidget( grp );
    _include = new QRadioButton( i18n("Include in .kim file"), grp );
    _manually = new QRadioButton( i18n("Manual copy next to .kim file"), grp );
    _auto = new QRadioButton( i18n("Automatically copy next to .kim file"), grp );
    _manually->setChecked( true );

    QString txt = i18n( "<qt><p>If your images are stored in a non-compressed file format then you may check this; "
                        "otherwise, this just wastes time during import and export operations.</p>"
                        "<p>In other words, do not check this if your images are stored in jpg, png or gif; but do check this "
                        "if your images are stored in tiff.</p></qt>" );
    QWhatsThis::add( _compress, txt );

    txt = i18n( "<qt><p>With this option you may limit the maximum dimensions (width and height) of your images. "
                "Doing so will make the resulting export file smaller, but will of course also make the quality "
                "worse if someone wants to see the exported images with larger dimensions.</p></qt>" );

    QWhatsThis::add( _enforeMaxSize, txt );
    QWhatsThis::add( _maxSize, txt );

    txt = i18n( "<qt><p>When exporting images, there are two things the person importing images needs:<br>"
                "1) meta information (who is on the images etc.)<br>"
                "2) the images themselves.<p>"

                "<p>The images themselves can either be placed next to the .kim file, or copied into the .kim file. "
                "Copying the images into the .kim file might be the right solution if you want to mail the images to someone "
                "who likely wants all images; on the other hand, if you put the images on the web, and a lot of people will "
                "see them but likely only download a few of them, then it is better to place the images next to the .kim "
                "file to avoid everyone having to download all the images (which is the case when they are in one big file.)</p></qt>" );
    QWhatsThis::add( grp, txt );
    QWhatsThis::add( _include, txt );
    QWhatsThis::add( _manually, txt );
    QWhatsThis::add( _auto, txt );

    setHelp( QString::fromLatin1( "chp-exportDialog" ) );
}

ImageFileLocation ExportConfig::imageFileLocation() const
{
    if ( _include->isChecked() )
        return Inline;
    else if ( _manually->isChecked() )
        return ManualCopy;
    else
        return AutoCopy;
}


Export::Export( const ImageInfoList& list, const QString& zipFile, bool compress, int maxSize, ImageFileLocation location,
                const QString& baseUrl, bool& ok )
    : _ok( ok ), _maxSize( maxSize ), _location( location )
{
    ok = true;
    _destdir = QFileInfo( zipFile ).dirPath();
    _zip = new KZip( zipFile );
    _zip->setCompression( compress ? KZip::DeflateCompression : KZip::NoCompression );
    if ( ! _zip->open( IO_WriteOnly ) ) {
        KMessageBox::error( 0, i18n("Error creating zip file") );
        ok = false;
        return;
    }

    // Create progress dialog
    int total = list.count(); // number of images *  create the thumbnails
    if ( location != ManualCopy )
        total *= 2;  // number of images *  copy images

    _steps = 0;
    _progressDialog = new QProgressDialog( QString::null, i18n("Cancel"), total, 0, "progress dialog", true );
    _progressDialog->setProgress( 0 );
    _progressDialog->show();

    _nameMap = Util::createUniqNameMap( list, false, QString::null );

    // Copy image files and generate thumbnails
    if ( location != ManualCopy ) {
        _copyingFiles = true;
        copyImages( list );
    }

    if ( _ok ) {
        _copyingFiles = false;
        generateThumbnails( list );
    }

    if ( _ok ) {
        // Create the index.xml file
        QCString indexml = createIndexXML( list, baseUrl );
        time_t t;
        time(&t);
        _zip->writeFile( QString::fromLatin1( "index.xml" ), QString::null, QString::null, indexml.size()-1,
                         0444, t, t, t, indexml.data() );

        _zip->close();
    }
}

QCString Export::createIndexXML( const ImageInfoList& list, const QString& baseUrl )
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );

    QDomElement top = doc.createElement( QString::fromLatin1( "KimDaBa-export" ) );
    top.setAttribute( QString::fromLatin1( "location" ),
                      _location == Inline ? QString::fromLatin1( "inline" ) : QString::fromLatin1( "external" ) );
    if ( !baseUrl.isEmpty() )
        top.setAttribute( QString::fromLatin1( "baseurl" ), baseUrl );
    doc.appendChild( top );


    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString file = (*it)->fileName();
        QString mappedFile = _nameMap[file];
        QDomElement elm = (*it)->save( doc );
        elm.setAttribute( QString::fromLatin1( "file" ), mappedFile );
        elm.setAttribute( QString::fromLatin1( "angle" ), 0 ); // We have rotated the image while copying it
        top.appendChild( elm );
    }
    return doc.toCString();
}



void Export::generateThumbnails( const ImageInfoList& list )
{
    _progressDialog->setLabelText( i18n("Creating thumbnails") );
    _loopEntered = false;
    _subdir = QString::fromLatin1( "Thumbnails/" );
    _filesRemaining = list.count(); // Used to break the event loop.
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        ImageManager::instance()->load( (*it)->fileName(), this, (*it)->angle(), 128, 128, false, true );
    }
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        qApp->eventLoop()->enterLoop();
    }
}

void Export::copyImages( const ImageInfoList& list )
{
    Q_ASSERT( _location != ManualCopy );

    _loopEntered = false;
    _subdir = QString::fromLatin1( "Images/" );

    _progressDialog->setLabelText( i18n("Copying image files") );

    _filesRemaining = 0;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString file =  (*it)->fileName();
        QString zippedName = _nameMap[file];

        if ( _maxSize == -1 ) {
            if ( QFileInfo( file ).isSymLink() )
                file = QFileInfo(file).readLink();

            if ( _location == Inline )
                _zip->addLocalFile( file, QString::fromLatin1( "Images/" ) + zippedName );
            else if ( _location == AutoCopy )
                Util::copy( file, _destdir + QString::fromLatin1( "/" ) + zippedName );
            _steps++;
            _progressDialog->setProgress( _steps );
        }
        else {
            _filesRemaining++;
            ImageManager::instance()->load( (*it)->fileName(), this, (*it)->angle(), _maxSize, _maxSize, false, true );
        }

        // Test if the cancel button was pressed.
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

#if QT_VERSION < 0x030104
        bool canceled = _progressDialog->wasCancelled();
#else
        bool canceled =  _progressDialog->wasCanceled();
#endif
        if ( canceled ) {
            _ok = false;
            return;
        }
    }
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        qApp->eventLoop()->enterLoop();
    }
}

void Export::pixmapLoaded( const QString& fileName, int /*width*/, int /*height*/, int /*angle*/, const QImage& image )
{
    // Add the file to the zip archive
    QString zipFileName = QString::fromLatin1( "%1/%2.%3" ).arg( Util::stripSlash(_subdir)).arg(QFileInfo( _nameMap[fileName] ).baseName())
                          .arg(QFileInfo( _nameMap[fileName] ).extension() );
    QByteArray data;
    QBuffer buffer( data );
    buffer.open( IO_WriteOnly );
    image.save( &buffer, KImageIO::type( zipFileName ).latin1() );

    if ( _location == Inline || !_copyingFiles )
        _zip->writeFile( zipFileName, QString::null, QString::null, data.size(), data );
    else {
        QString file = _destdir + QString::fromLatin1( "/" ) + _nameMap[fileName];
        QFile out( file );
        if ( !out.open( IO_WriteOnly ) ) {
            KMessageBox::error( 0, i18n("Error writing file %1").arg( file ) );
            _ok = false;
        }
        out.writeBlock( data, data.size() );
        out.close();
    }

    qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

#if QT_VERSION < 0x030104
    bool canceled = (!_ok || _progressDialog->wasCancelled());
#else
    bool canceled = (!_ok ||  _progressDialog->wasCanceled());
#endif

    if ( canceled ) {
        _ok = false;
        qApp->eventLoop()->exitLoop();
        ImageManager::instance()->stop( this );
        return;
    }

    _steps++;
    _filesRemaining--;
    _progressDialog->setProgress( _steps );

    if ( _filesRemaining == 0 )
        qApp->eventLoop()->exitLoop();
}

void Export::showUsageDialog()
{
    QString txt =
        i18n( "<qt><p>Other KimDaBa users may now load the import file into their database, by choosing <tt>import</tt> in "
              "the file menu.</p>"
              "<p>If they find it on a web site, and the web server is correctly configured, all they need to do is simply "
              "to click it from within konqueror. To enable this, your web server needs to be configured for KimDaBa. You do so by adding "
              "the following line to <tt>/etc/httpd/mime.types</tt> or similar:"
              "<pre>application/vnd.kde.kimdaba-import kim</pre>"
              "This will make your web server tell konqueror that it is a KimDaBa fill when clicking on the link, "
              "otherwise the web server will just tell konqueror that it is a plain text file.</p></qt>" );

    KMessageBox::information( 0, txt, i18n("How to use the export file"), QString::fromLatin1("export_how_to_use_the_export_file") );
}




#include "export.moc"
