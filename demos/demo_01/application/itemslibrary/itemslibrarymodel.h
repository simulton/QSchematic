#pragma once

#include <QAbstractItemModel>
#include "itemslibrarymodelitem.h"

namespace QSchematic {
    class Item;
}

class ItemsLibraryModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ItemsLibraryModel)

public:
    enum LibraryItems {
        Root,
        RootOperations,
        Operation,
        RootFlows,
        Flow
    };
    Q_ENUM(LibraryItems)

    explicit ItemsLibraryModel(QObject* parent = nullptr);
    virtual ~ItemsLibraryModel() override;

    const QSchematic::Item* itemFromIndex(const QModelIndex& index) const;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

private:
    typedef LibraryItems itemType;

    void createModel();
    void addTreeItem(const QString& name, const QIcon& icon, const QSchematic::Item* item, ItemsLibraryModelItem<itemType>* parent);

    ItemsLibraryModelItem<itemType>* _rootItem;
};
