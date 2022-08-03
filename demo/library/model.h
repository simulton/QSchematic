#pragma once

#include "model_item.h"
#include "iteminfo.h"

#include <QAbstractItemModel>

namespace QSchematic
{
    class Item;
}

namespace Library
{

    class Model :
        public QAbstractItemModel
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Model)

    public:
        enum LibraryItems
        {
            Root,
            RootOperations,
            Operation,
            RootFlows,
            Flow,
            RootBascis,
            Basic
        };
        Q_ENUM(LibraryItems)

        using model_item = ModelItem<LibraryItems, ItemInfo>;

        explicit Model(QObject* parent = nullptr);
        ~Model() override;

        [[nodiscard]]
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

        void createModel();
        void addTreeItem(const QString& name, const QIcon& icon, const QSchematic::Item* item, model_item* parent);

        model_item* _rootItem = nullptr;
    };

}
