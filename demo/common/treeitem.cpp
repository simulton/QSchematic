#include "treeitem.hpp"

TreeItem::TreeItem(const QVector<QVariant>& data, TreeItem* parent) :
    _data(data),
    _parent(parent)
{
}

TreeItem::~TreeItem()
{
    qDeleteAll(_children);
}

void TreeItem::clear()
{
    qDeleteAll(_children);
    _children.clear();
}

void TreeItem::setData(QVector<QVariant>&& data)
{
    _data = std::move(data);
}

void TreeItem::appendChild(TreeItem* item)
{
    _children.append(item);
    item->_parent = this;
}

TreeItem* TreeItem::child(int row)
{
    if (row < 0 || row >= _children.size())
        return nullptr;

    return _children.at(row);
}

int TreeItem::childCount() const
{
    return _children.count();
}

int TreeItem::columnCount() const
{
    return _data.count();
}

QVariant TreeItem::data(int column) const
{
    if (column < 0 || column >= _data.size())
        return QVariant();

    return _data.at( column );
}

TreeItem* TreeItem::parentItem()
{
    return _parent;
}

int TreeItem::row() const
{
    if ( _parent )
        return _parent->_children.indexOf( const_cast<TreeItem*>( this ) );

    return 0;
}
