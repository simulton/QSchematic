#include <QJsonObject>
#include "itemfactory.h"
#include "node.h"
#include "wire.h"
#include "connector.h"
#include "label.h"

using namespace QSchematic;

ItemFactory& ItemFactory::instance()
{
    static ItemFactory instance;

    return instance;
}

void ItemFactory::setCustomItemsFactory(const std::function<std::unique_ptr<Item>(const QJsonObject&)>& factory)
{
    _customItemFactory = factory;
}

std::unique_ptr<Item> ItemFactory::fromJson(const QJsonObject& object) const
{
    // Extract the type
    Item::ItemType type = ItemFactory::extractType(object);

    // Create the item
    std::unique_ptr<Item> item;

    // First, try custom types
    if (_customItemFactory) {
        item.reset(_customItemFactory(object).release());
    }

    // Fall back to internal types
    if (!item) {
        switch (type) {
        case Item::NodeType:
            item.reset(new Node);
            break;

        case Item::WireType:
            item.reset(new Wire);
            break;

        case Item::ConnectorType:
            item.reset(new Connector);
            break;

        case Item::LabelType:
            item.reset(new Label);
            break;

        case Item::QSchematicItemUserType:
            break;
        }
    }

    return item;
}

Item::ItemType ItemFactory::extractType(const QJsonObject& object)
{
    return static_cast<Item::ItemType>(object["item type id"].toInt());
}
