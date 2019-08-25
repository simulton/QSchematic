#pragma once

#include <QVector>
#include <QVariant>

#include <QVariant>
#include <QVector>

class TreeItem
{
public:
    explicit TreeItem( const QVector<QVariant>& data, TreeItem* parentItem = nullptr );
    ~TreeItem();

    void clear();
    void setData( QVector<QVariant>&& data );
    void appendChild( TreeItem* child );

    TreeItem* child( int row );
    int childCount( ) const;
    int columnCount( ) const;
    QVariant data( int column ) const;
    int row( ) const;
    TreeItem* parentItem( );

private:
    QVector<TreeItem*> _children;
    QVector<QVariant> _data;
    TreeItem* _parent;
};
