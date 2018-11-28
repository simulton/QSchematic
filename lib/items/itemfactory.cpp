#include <QJsonObject>
#include "itemfactory.h"
#include "node.h"

using namespace QSchematic;

std::unique_ptr<Item> ItemFactory::fromJson(const QJsonObject& object)
{
    // Extract the type
    Item::ItemType type = ItemFactory::extractType(object);

    // Create the item
    std::unique_ptr<Item> item;
    switch (type) {
    case Item::NodeType:
        item.reset(new Node);
        break;
    }

    // Sanity check
    if (!item) {
        return nullptr;
    }

    // Load
    item->fromJson(object);

    return item;
}

Item::ItemType ItemFactory::extractType(const QJsonObject& object)
{
    Q_UNUSED(object)

#warning ToDo: Implement me

    return Item::NodeType;
}
