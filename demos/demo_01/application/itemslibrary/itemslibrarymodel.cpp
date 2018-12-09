#include <QColor>
#include <QMimeData>
#include "itemslibrarymodel.h"
#include "itemslibrarymodelitem.h"
#include "iteminfo.h"
#include "../items/operation.h"
#include "../../../lib/items/item.h"
#include "../../../lib/items/itemmimedata.h"

ItemsLibraryModel::ItemsLibraryModel(QObject* parent) : QAbstractItemModel(parent)
{
    _rootItem = new ItemsLibraryModelItem<itemType>(Root, nullptr);

    createModel();
}

ItemsLibraryModel::~ItemsLibraryModel()
{
    delete _rootItem;
}

void ItemsLibraryModel::createModel()
{
    // Purge the old (current) model
    while (_rootItem->childCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, 0);
        _rootItem->deleteChild(0);
        endRemoveRows();
    }

    // Root operations
    ItemsLibraryModelItem<itemType>* rootOperations = new ItemsLibraryModelItem<itemType>(RootOperations, nullptr, _rootItem);
    beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());
    _rootItem->appendChild(rootOperations);
    endInsertRows();

    // Operations
    addTreeItem("Generic Operation", QIcon(), new ::Operation, rootOperations);
}

void ItemsLibraryModel::addTreeItem(const QString& name, const QIcon& icon, const QSchematic::Item* item, ItemsLibraryModelItem<itemType>* parent)
{
    auto newItem = new ItemsLibraryModelItem<itemType>(Operation, new ItemInfo(name, icon, item), parent);
    beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount());
    parent->appendChild(newItem);
    endInsertRows();
}

const QSchematic::Item* ItemsLibraryModel::itemFromIndex(const QModelIndex& index) const
{
    // Retrieve the item
    ItemsLibraryModelItem<itemType>* modelItem = static_cast<ItemsLibraryModelItem<itemType>*>(index.internalPointer());
    if (!modelItem) {
        return nullptr;
    }

    // Retrieve the item info
    ItemInfo* itemInfo = static_cast<ItemInfo*>(modelItem->data());
    if (!itemInfo) {
        return nullptr;
    }

    // Retrieve the QSchematic::Item
    auto item = static_cast<const QSchematic::Item*>(itemInfo->item);
    if (!item) {
        return nullptr;
    }

    return item;
}

QModelIndex ItemsLibraryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    ItemsLibraryModelItem<itemType>* parentItem;
    if (!parent.isValid()) {
        parentItem = _rootItem;
    } else {
        parentItem = static_cast<ItemsLibraryModelItem<itemType>*>(parent.internalPointer());
    }

    ItemsLibraryModelItem<itemType>* childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex ItemsLibraryModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    ItemsLibraryModelItem<itemType>* childItem = static_cast<ItemsLibraryModelItem<itemType>*>(index.internalPointer());
    ItemsLibraryModelItem<itemType>* parentItem = childItem->parent();

    if (parentItem == _rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int ItemsLibraryModel::rowCount(const QModelIndex& parent) const
{
    ItemsLibraryModelItem<itemType>* parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = _rootItem;
    } else {
        parentItem = static_cast<ItemsLibraryModelItem<itemType>*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int ItemsLibraryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant ItemsLibraryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    // Retrieve the model item
    ItemsLibraryModelItem<itemType>* modelItem = static_cast<ItemsLibraryModelItem<itemType>*>(index.internalPointer());
    if (!modelItem) {
        return QVariant();
    }

    // Retrieve the ItemInfo
    const ItemInfo* itemInfo = reinterpret_cast<const ItemInfo*>(modelItem->data());

    // Return the appropriate data
    switch (modelItem->type()) {
    case ItemsLibraryModel::RootOperations:
    {
        switch (role) {
        case Qt::DisplayRole:
            return "Operations";
        }
    }

    case ItemsLibraryModel::Operation:
    {
        switch (role) {
        case Qt::DisplayRole:
            Q_ASSERT(itemInfo);
            return itemInfo->name;
        }
    }

    default:
        return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags ItemsLibraryModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    // Default flags
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // Add item specific flags
    ItemsLibraryModelItem<itemType>* modelItem = static_cast<ItemsLibraryModelItem<itemType>*>(index.internalPointer());
    if (!modelItem) {
        return Qt::NoItemFlags;
    }
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

QStringList ItemsLibraryModel::mimeTypes() const
{

    return { QSchematic::MIME_TYPE_NODE };
}

QMimeData* ItemsLibraryModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.count() != 1)
        return new QMimeData();

    const QModelIndex& index = indexes.first();
    if (!index.isValid())
        return new QMimeData();

    ItemsLibraryModelItem<itemType>* modelItem = static_cast<ItemsLibraryModelItem<itemType>*>(index.internalPointer());
    if (!modelItem) {
        return new QMimeData();
    }
    switch (modelItem->type()) {
    case Operation:
    {
        // Retrieve the widget info
        auto itemInfo = static_cast<ItemInfo*>(modelItem->data());
        if (!itemInfo) {
            return new QMimeData();
        }

        // Clone the item
        auto itemClone = itemInfo->item->deepCopy();

        return new QSchematic::ItemMimeData(std::move(itemClone));
    }
        break;

    default:
        break;
    }

    return new QMimeData();
}
