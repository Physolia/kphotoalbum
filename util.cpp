#include "util.h"
#include "options.h"
#include "imageinfo.h"
#include <klocale.h>
#include <qfileinfo.h>
#include <kmessagebox.h>
#include <qurl.h>
#include <kapplication.h>

bool Util::writeOptions( QDomDocument doc, QDomElement elm, QMap<QString, QStringList>& options,
                         QMap<QString,Options::OptionGroupInfo>* optionGroupInfo )
{
    bool anyAtAll = false;
    QStringList grps = Options::instance()->optionGroups();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        QString name = *it;
        opt.setAttribute( QString::fromLatin1("name"),  name );

        if ( optionGroupInfo ) {
            opt.setAttribute( QString::fromLatin1( "text" ), (*optionGroupInfo)[name]._text );
            opt.setAttribute( QString::fromLatin1( "icon" ), (*optionGroupInfo)[name]._icon );
            opt.setAttribute( QString::fromLatin1( "show" ), (*optionGroupInfo)[name]._show );
        }

        QStringList list = options[name];
        bool any = false;
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
            opt.appendChild( val );
            any = true;
            anyAtAll = true;
        }
        if ( any || optionGroupInfo  ) // We always want to write all records when writing from Options
            elm.appendChild( opt );
    }
    return anyAtAll;
}



void Util::readOptions( QDomElement elm, QMap<QString, QStringList>* options,
                        QMap<QString,Options::OptionGroupInfo>* optionGroupInfo )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "options" ) );

    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull();
          nodeOption = nodeOption.nextSibling() )  {

        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            Q_ASSERT( elmOption.tagName() == QString::fromLatin1("option") );
            QString name = elmOption.attribute( QString::fromLatin1("name") );
            if ( !name.isNull() )  {
                // Read Option Group info
                if ( optionGroupInfo ) {
                    QString text= elmOption.attribute( QString::fromLatin1("text"), name );
                    QString icon= elmOption.attribute( QString::fromLatin1("icon") );
                    bool show = (bool) elmOption.attribute( QString::fromLatin1( "show" ),
                                                            QString::fromLatin1( "1" ) ).toInt();
                    (*optionGroupInfo)[name] = Options::OptionGroupInfo( text, icon, show );
                }

                // Read values
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull();
                      nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == QString::fromLatin1("value") );
                        QString value = elmValue.attribute( QString::fromLatin1("value") );
                        if ( !value.isNull() )  {
                            (*options)[name].append( value );
                        }
                    }
                }
            }
        }
    }
}

QString Util::createInfoText( ImageInfo* info, QMap< int,QPair<QString,QString> >* linkMap )
{
    Q_ASSERT( info );
    QString text;
    if ( Options::instance()->showDate() )  {
        if ( info->startDate().isNull() ) {
            // Don't append anything
        }
        else if ( info->endDate().isNull() )
            text += info->startDate();
        else
            text += info->startDate() + i18n(" to ") + info->endDate();

        if ( !text.isEmpty() ) {
            text = i18n("<b>Date: </b> ") + text + QString::fromLatin1("<br>");
        }
    }

    QStringList grps = Options::instance()->optionGroups();
    int link = 0;
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QString optionGroup = *it;
        if ( Options::instance()->showOption( optionGroup ) ) {
            QStringList items = info->optionValue( optionGroup );
            if (items.count() != 0 ) {
                text += QString::fromLatin1( "<b>%1: </b> " )
                        .arg( Options::instance()->textForOptionGroup( optionGroup ) );
                bool first = true;
                for( QStringList::Iterator it2 = items.begin(); it2 != items.end(); ++it2 ) {
                    QString item = *it2;
                    if ( first )
                        first = false;
                    else
                        text += QString::fromLatin1( ", " );

                    if ( linkMap ) {
                        ++link;
                        (*linkMap)[link] = QPair<QString,QString>( optionGroup, item );
                        text += QString::fromLatin1( "<a href=\"%1\">%2</a>")
                                .arg( link ).arg( item );
                    }
                    else
                        text += item;
                }
                text += QString::fromLatin1( "<br>" );
            }
        }
    }

    if ( Options::instance()->showDescription() && !info->description().isEmpty())  {
        if ( !text.isEmpty() )
            text += i18n("<b>Description: </b> ") +  info->description() + QString::fromLatin1("<br>");
    }

    return text;
}

void Util::checkForBackupFile( const QString& realName, const QString& backupName )
{
    QFileInfo backUpFile( backupName);
    QFileInfo indexFile( realName );
    if ( !backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() )
        return;

    int code = KMessageBox::questionYesNo( 0, i18n("Backup file '%1' exists and is newer than '%2'. "
                                                      "Should I use the backup file?")
                                           .arg(backupName).arg(realName),
                                           i18n("Found Backup File") );
    if ( code == KMessageBox::Yes ) {
        QFile in( backupName );
        if ( in.open( IO_ReadOnly ) ) {
            QFile out( realName );
            if (out.open( IO_WriteOnly ) ) {
                char data[1024];
                int len;
                while ( (len = in.readBlock( data, 1024 ) ) )
                    out.writeBlock( data, len );
            }
        }
    }
}

bool Util::ctrlKeyDown()
{
    return KApplication::keyboardModifiers() & KApplication::ControlModifier;
}

