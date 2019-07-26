#include "itemfactory.h"
#include "node.h"
#include "wire.h"
#include "wireroundedcorners.h"
#include "splinewire.h"
#include "connector.h"
#include "label.h"

using namespace QSchematic;

ItemFactory& ItemFactory::instance()
{
    static ItemFactory instance;

    return instance;
}

void ItemFactory::setCustomItemsFactory(const std::function<std::unique_ptr<Item>(const Gpds::Container&)>& factory)
{
    _customItemFactory = factory;
}

std::unique_ptr<Item> ItemFactory::fromContainer(const Gpds::Container& container) const
{
    // Extract the type
    Item::ItemType type = ItemFactory::extractType(container);

    // Create the item
    std::unique_ptr<Item> item;

    // First, try custom types
    if (_customItemFactory) {
        item.reset(_customItemFactory(container).release());
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

        case Item::WireRoundedCornersType:
            item.reset(new WireRoundedCorners);
            break;

        case Item::SplineWireType:
            item.reset(new SplineWire);
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

Item::ItemType ItemFactory::extractType(const Gpds::Container& container)
{
    return static_cast<Item::ItemType>( container.getAttribute<int>( "type_id" ).value_or( -1 ) );
}
