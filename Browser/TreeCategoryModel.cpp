#include "TreeCategoryModel.h"
#include <DB/ImageDB.h>
#include <DB/CategoryItem.h>

struct Data
{
    Data( const QString& name )
        : name( name ), parent(0) {}
    ~Data() {
        qDeleteAll( children );
    }
    void addChild( Data* child )
    {
        child->parent = this;
        children.append(child);
    }


    QString name;
    QList<Data*> children;
    Data* parent;
};

TreeCategoryModel::TreeCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info )
    : AbstractCategoryModel( category, info )
{
    _data = new Data( QString() );
    createData(  _category->itemsCategories().data(), 0 );
    if ( hasNoneEntry() ) {
        Data* data = new Data( DB::ImageDB::NONE() );
        data->parent = _data;
        _data->children.prepend( data );
    }
}

int TreeCategoryModel::rowCount( const QModelIndex& index ) const
{
    return indexToData(index)->children.count();
}

int TreeCategoryModel::columnCount( const QModelIndex& ) const
{
    return 3;
}

QModelIndex TreeCategoryModel::index( int row, int column, const QModelIndex & parent ) const
{
    if ( indexToData(parent)->children.count() < row )
        return QModelIndex();

    return createIndex( row, column, indexToData(parent)->children[row] );
}

QModelIndex TreeCategoryModel::parent( const QModelIndex & index ) const
{
    Data* me = indexToData( index );
    if ( me == _data )
        return QModelIndex();

    Data* parent = me->parent;
    if ( parent == _data )
        return QModelIndex();

    Data* grandParent = parent->parent;

    return createIndex( grandParent->children.indexOf( parent ), 0, parent );
}

TreeCategoryModel::~TreeCategoryModel()
{
    delete _data;
}

bool TreeCategoryModel::createData( DB::CategoryItem* parentCategoryItem, Data* parent )
{
    const QString name = parentCategoryItem->_name;
    const int imageCount = _images.contains(name) ? _images[name] : 0;
    const int videoCount = _videos.contains(name) ? _videos[name] : 0;

    Data* myData = new Data( name );
    bool anyItems = imageCount != 0 || videoCount != 0;

    for( QList<DB::CategoryItem*>::ConstIterator subCategoryIt = parentCategoryItem->_subcategories.constBegin();
         subCategoryIt != parentCategoryItem->_subcategories.constEnd(); ++subCategoryIt ) {
        anyItems = createData( *subCategoryIt, myData ) || anyItems;
    }

    if ( parent ) {
        if ( anyItems )
            parent->addChild( myData );
        else
            delete myData;
    }
    else {
        _data = myData;
    }

    return anyItems;

}

Data* TreeCategoryModel::indexToData( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return _data;
    else
        return static_cast<Data*>( index.internalPointer() );
}

QString TreeCategoryModel::indexToName(const QModelIndex& index ) const
{
    const Data* data = indexToData( index );
    return data->name;

}
