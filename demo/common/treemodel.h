#pragma once

#include <QAbstractItemModel>

class TreeItem;

class TreeModel :
    public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TreeModel)

public:
    explicit TreeModel(QObject* parent = nullptr);
    ~TreeModel() override;

    void clear();

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;

protected:
    TreeItem* _rootItem = nullptr;
};
