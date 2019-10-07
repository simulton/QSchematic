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

void ItemFactory::setCustomItemsFactory(const std::function<std::shared_ptr<Item>(const Gpds::Container&)>& factory)
{
    _customItemFactory = factory;
}

std::shared_ptr<Item> ItemFactory::fromContainer(const Gpds::Container& container) const
{
    // First, try custom types
    if (_customItemFactory) {
        if (auto item = _customItemFactory(container)) { //.release();
            return item;
        }
    }

    // Extract the type
    Item::ItemType type = ItemFactory::extractType(container);

    // Fall back to internal types
    switch (type) {
    case Item::NodeType:
        return std::make_shared<Node>();

    case Item::WireType:
        return std::make_shared<Wire>();

    case Item::WireRoundedCornersType:
        return std::make_shared<WireRoundedCorners>();

    case Item::SplineWireType:
        return std::make_shared<SplineWire>();

    case Item::ConnectorType:
        return std::make_shared<Connector>();

    case Item::LabelType:
        return std::make_shared<Label>();

    case Item::QSchematicItemUserType:
        return {};
    }

    return {};
}

Item::ItemType ItemFactory::extractType(const Gpds::Container& container)
{
    return static_cast<Item::ItemType>( container.getAttribute<int>( "type_id" ).value_or( -1 ) );
}
