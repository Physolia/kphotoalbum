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
#include "DraggableListView.h"
#include "DragObject.h"
#include "DB/Category.h"
#include "CheckDropItem.h"

CategoryListView::DraggableListView::DraggableListView( const DB::CategoryPtr& category, QWidget* parent )
    :QTreeWidget( parent ), _category( category )
{
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(ExtendedSelection);
}

#ifdef COMMENTED_OUT_DURING_PORTING
Q3DragObject* CategoryListView::DraggableListView::dragObject()
{
    CategoryListView::DragItemInfoSet selected;
    for ( Q3ListViewItemIterator itemIt( this ); *itemIt; ++itemIt ) {
        if ( (*itemIt)->isSelected() ) {
            Q3ListViewItem* parent = (*itemIt)->parent();
            QString parentText = parent ? parent->text(0) : QString();
            selected.insert( CategoryListView::DragItemInfo( parentText, (*itemIt)->text(0) ) );
        }
    }

    return new DragObject( selected, this );
}
#endif // COMMENTED_OUT_DURING_PORTING

DB::CategoryPtr CategoryListView::DraggableListView::category() const
{
    return _category;
}

void CategoryListView::DraggableListView::emitItemsChanged()
{
    emit itemsChanged();
}

QMimeData *CategoryListView::DraggableListView::mimeData(const QList<QTreeWidgetItem *> items) const
{
    CategoryListView::DragItemInfoSet selected;
    Q_FOREACH( QTreeWidgetItem* item, items ) {
        QTreeWidgetItem* parent = item->parent();
        QString parentText = parent ? parent->text(0) : QString();
        selected.insert( CategoryListView::DragItemInfo( parentText, item->text(0) ) );
    }

    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << selected;

    QMimeData* mime = new QMimeData;
    mime->setData(QString::fromUtf8("x-kphotoalbum/x-categorydrag"), data);
    return mime;
}

QStringList CategoryListView::DraggableListView::mimeTypes() const
{
    return QStringList(QString::fromUtf8("x-kphotoalbum/x-categorydrag"));
}

bool CategoryListView::DraggableListView::dropMimeData(QTreeWidgetItem *parent, int, const QMimeData *data, Qt::DropAction )
{
    return static_cast<CheckDropItem*>(parent)->dataDropped(data);
}

#include "DraggableListView.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
