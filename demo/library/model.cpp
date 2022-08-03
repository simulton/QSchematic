#include "model.h"
#include "iteminfo.h"
#include "../items/operation.h"
#include "../items/operationdemo1.h"
#include "../items/flowstart.h"
#include "../items/flowend.h"

#include <qschematic/items/itemmimedata.h>
#include <qschematic/items/label.h>

#include <QMimeData>

using namespace Library;

Model::Model(QObject* parent) :
    QAbstractItemModel(parent)
{
    _rootItem = new model_item(Root, nullptr);

    createModel();
}

Model::~Model()
{
    delete _rootItem;
}

void Model::createModel()
{
    // Purge the old (current) model
    while (_rootItem->childCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, 0);
        _rootItem->deleteChild(0);
        endRemoveRows();
    }

    // Root operations
    auto rootOperations = new model_item(RootOperations, nullptr, _rootItem);
    beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());
    _rootItem->appendChild(rootOperations);
    endInsertRows();

    // Root flows
    auto rootFlows = new model_item(RootFlows, nullptr, _rootItem);
    beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());
    _rootItem->appendChild(rootFlows);
    endInsertRows();

    // Root basics
    auto rootBasics = new model_item(RootBascis, nullptr, _rootItem);
    beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());
    _rootItem->appendChild(rootBasics);
    endInsertRows();

    // Operations
    addTreeItem("Generic", QIcon(), new ::Operation, rootOperations);
    addTreeItem("Demo 1", QIcon(), new ::OperationDemo1, rootOperations);

    // Flows
    addTreeItem("Start", QIcon(), new ::FlowStart, rootFlows);
    addTreeItem("End", QIcon(), new ::FlowEnd, rootFlows);

    // Basics
    auto label = new QSchematic::Label;
    label->setHasConnectionPoint(false);
    label->setText(QStringLiteral("Label"));
    addTreeItem("Label", QIcon(), label, rootBasics);
}

void Model::addTreeItem(const QString& name, const QIcon& icon, const QSchematic::Item* item, model_item* parent)
{
    auto newItem = new model_item(Operation, new ItemInfo(name, icon, item), parent);
    beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());
    parent->appendChild(newItem);
    endInsertRows();
}

const QSchematic::Item* Model::itemFromIndex(const QModelIndex& index) const
{
    // Retrieve the item
    auto modelItem = static_cast<model_item*>(index.internalPointer());
    if (!modelItem) {
        return nullptr;
    }

    // Retrieve the item info
    auto itemInfo = static_cast<const ItemInfo*>(modelItem->data());
    if (!itemInfo) {
        return nullptr;
    }

    // Retrieve the QSchematic::Item
    return itemInfo->item;
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return { };

    model_item* parentItem;
    if (!parent.isValid()) {
        parentItem = _rootItem;
    } else {
        parentItem = static_cast<model_item*>(parent.internalPointer());
    }

    model_item* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return { };
}

QModelIndex Model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return { };

    auto childItem = static_cast<model_item*>(index.internalPointer());
    auto parentItem = childItem->parent();

    if (parentItem == _rootItem)
        return { };

    return createIndex(parentItem->row(), 0, parentItem);
}

int Model::rowCount(const QModelIndex& parent) const
{
    model_item* parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = _rootItem;
    } else {
        parentItem = static_cast<model_item*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int Model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 1;
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return { };

    // Retrieve the model item
    auto modelItem = static_cast<model_item*>(index.internalPointer());
    if (!modelItem)
        return { };

    // Retrieve the ItemInfo
    const auto itemInfo = reinterpret_cast<const ItemInfo*>(modelItem->data());

    // Return the appropriate data
    switch (modelItem->type()) {
        case Model::RootOperations: {
            switch (role) {
                case Qt::DisplayRole:
                    return "Operations";

                default:
                    return {};
            }
        }

        case Model::Operation: {
            switch (role) {
                case Qt::DisplayRole:
                    Q_ASSERT(itemInfo);
                    return itemInfo->name;

                default:
                    return {};
            }
        }

        case Model::RootFlows: {
            switch (role) {
                case Qt::DisplayRole:
                    return "Flows";

                default:
                    return {};
            }
        }

        case Model::Flow: {
            switch (role) {
                case Qt::DisplayRole:
                    Q_ASSERT(itemInfo);
                    return itemInfo->name;

                default:
                    return {};
            }
        }

        case Model::RootBascis: {
            switch (role) {
                case Qt::DisplayRole:
                    return "Basics";

                default:
                    return {};
            }
        }

        case Model::Basic: {
            switch (role) {
                case Qt::DisplayRole:
                    Q_ASSERT(itemInfo);
                    return itemInfo->name;

                default:
                    return {};
            }
        }

        default:
            return {};

    }
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    // Default flags
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // Add item specific flags
    auto modelItem = static_cast<model_item*>(index.internalPointer());
    if (!modelItem)
        return Qt::NoItemFlags;

    switch (modelItem->type()) {
        case RootOperations:
            flags &= ~Qt::ItemIsSelectable;
            break;

        default:
            flags |= Qt::ItemIsDragEnabled;
            break;
    }

    return flags;
}

QStringList Model::mimeTypes() const
{
    return { QSchematic::MIME_TYPE_NODE };
}

QMimeData* Model::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.count() != 1)
        return new QMimeData();

    const QModelIndex& index = indexes.first();
    if (!index.isValid())
        return new QMimeData();

    auto modelItem = static_cast<model_item*>(index.internalPointer());
    if (!modelItem)
        return new QMimeData();

    switch (modelItem->type()) {
        case Operation:
        {
            // Retrieve the widget info
            auto itemInfo = static_cast<const ItemInfo*>(modelItem->data());
            if (!itemInfo) {
                return new QMimeData();
            }

            // Clone the item
            auto itemClone = itemInfo->item->deepCopy();

            return new QSchematic::ItemMimeData(std::move(itemClone));
        }

        default:
            break;
    }

    return new QMimeData();
}
