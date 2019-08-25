#pragma once

#include <QAbstractItemModel>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY( TreeModel )

public:
    explicit TreeModel( QObject* parent = nullptr );
    virtual ~TreeModel( ) override;

    void clear();

    virtual QVariant data( const QModelIndex& index, int role ) const override;
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const override;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    virtual QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    virtual QModelIndex parent( const QModelIndex& index ) const override;
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const override;

protected:
    TreeItem* _rootItem;
};
