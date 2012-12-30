/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef CATEGORYLISTVIEW_CHECKDROPITEM_H
#define CATEGORYLISTVIEW_CHECKDROPITEM_H

#include "DragItemInfo.h"
#include <QTreeWidgetItem>
#include <QMimeData>

class QDropEvent;
namespace CategoryListView
{
class DraggableListView;

class CheckDropItem :public QTreeWidgetItem
{
public:
    CheckDropItem( DraggableListView* listview, const QString& column1, const QString& column2 );
    CheckDropItem( DraggableListView* listview, QTreeWidgetItem* parent, const QString& column1, const QString& column2 );
#ifdef COMMENTED_OUT_DURING_PORTING
    virtual bool acceptDrop( const QMimeSource* mime ) const;
#endif // COMMENTED_OUT_DURING_PORTING
    void setDNDEnabled( bool );
    bool dataDropped( const QMimeData* data );

protected:
    bool isSelfDrop( const QString& parent, const DragItemInfoSet& children ) const;
    bool verifyDropWasIntended( const QString& parent, const DragItemInfoSet& children );
    DragItemInfoSet extractData( const QMimeData* data ) const;
    virtual void activate();

private:
    DraggableListView* _listView;
};

}

#endif /* CATEGORYLISTVIEW_CHECKDROPITEM_H */
