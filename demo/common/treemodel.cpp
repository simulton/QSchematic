#include "treemodel.h"
#include "treeitem.h"

TreeModel::TreeModel( QObject* parent ) :
    QAbstractItemModel( parent ),
    _rootItem( nullptr )
{
    _rootItem = new TreeItem( { } );
}

TreeModel::~TreeModel()
{
    delete _rootItem;
}

void TreeModel::clear()
{
    Q_ASSERT( _rootItem );

    if ( _rootItem->childCount() <= 0 ) {
        return;
    }

    beginRemoveRows( QModelIndex(), 0, _rootItem->childCount()-1 );
    _rootItem->clear();
    endRemoveRows();
}

int TreeModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.column() > 0 ) {
        return 0;
    }

    TreeItem* parentItem;
    if ( not parent.isValid() ) {
        parentItem = _rootItem;
    } else {
        parentItem = static_cast<TreeItem*>( parent.internalPointer() );
    }

    return parentItem->childCount();
}

int TreeModel::columnCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    }

    Q_ASSERT( _rootItem );
    return _rootItem->columnCount();
}

Qt::ItemFlags TreeModel::flags( const QModelIndex& index ) const
{
    if ( not index.isValid() ) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QModelIndex TreeModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( not hasIndex(row, column, parent) ) {
        return QModelIndex();
    }

    TreeItem* parentItem;
    if ( not parent.isValid() ) {
        parentItem = _rootItem;
    } else {
        parentItem = static_cast<TreeItem*>( parent.internalPointer() );
    }

    TreeItem* childItem = parentItem->child( row );
    if ( childItem ) {
        return createIndex( row, column, childItem );
    }

    return QModelIndex();
}

QModelIndex TreeModel::parent( const QModelIndex& index ) const
{
    if ( not index.isValid() ) {
        return QModelIndex();
    }

    TreeItem* childItem = static_cast<TreeItem*>( index.internalPointer() );
    Q_ASSERT( childItem );
    TreeItem* parentItem = childItem->parentItem();
    Q_ASSERT( parentItem );

    if ( parentItem == _rootItem ) {
        return QModelIndex();
    }

    return createIndex( parentItem->row(), 0, parentItem );
}

QVariant TreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal and role == Qt::DisplayRole ) {
        return _rootItem->data( section );
    }

    return QVariant();
}

QVariant TreeModel::data( const QModelIndex& index, int role ) const
{
    if ( not index.isValid() ) {
        return QVariant();
    }

    if ( role != Qt::DisplayRole ) {
        return QVariant();
    }

    TreeItem* item = static_cast<TreeItem*>( index.internalPointer() );

    return item->data( index.column() );
}
