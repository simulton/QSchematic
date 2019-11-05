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

void ItemFactory::setCustomItemsFactory(const std::function<OriginMgrT<Item>(const Gpds::Container&)>& factory)
{
    _customItemFactory = factory;
}

OriginMgrT<Item> ItemFactory::fromContainer(const Gpds::Container& container) const
{
    // Extract the type
    Item::ItemType type = ItemFactory::extractType(container);

    // First, try custom types
    if (_customItemFactory) {
        if (auto item = _customItemFactory(container)) {
            return item;
        }
    }

    // Fall back to internal types
    switch (type) {
    case Item::NodeType:
        return QSchematic::make_origin<Node>();

    case Item::WireType:
        return QSchematic::make_origin<Wire>();

    case Item::WireRoundedCornersType:
        return QSchematic::make_origin<WireRoundedCorners>();

    case Item::SplineWireType:
        return QSchematic::make_origin<SplineWire>();

    case Item::ConnectorType:
        return QSchematic::make_origin<Connector>();

    case Item::LabelType:
        return QSchematic::make_origin<Label>();

    case Item::QSchematicItemUserType:
        break;
    }

    return {};
}

Item::ItemType ItemFactory::extractType(const Gpds::Container& container)
{
    return static_cast<Item::ItemType>( container.getAttribute<int>( "type_id" ).value_or( -1 ) );
}
