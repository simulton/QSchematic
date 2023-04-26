#include "itemfactory.h"
#include "node.h"
#include "wire.h"
#include "wireroundedcorners.h"
#include "splinewire.h"
#include "connector.h"
#include "label.h"

using namespace QSchematic::Items;

Factory&
Factory::instance()
{
    static Factory instance;

    return instance;
}

void
Factory::setCustomItemsFactory(const std::function<std::shared_ptr<Item>(const gpds::container&)>& factory)
{
    _customItemFactory = factory;
}

std::shared_ptr<Item>
Factory::from_container(const gpds::container& container) const
{
    // First, try custom types
    if (_customItemFactory) {
        if (auto item = _customItemFactory(container); item)
            return item;
    }

    // Extract the type
    auto type = Factory::extractType(container);

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
        break;
    }

    return { };
}

Item::ItemType
Factory::extractType(const gpds::container& container)
{
    return static_cast<Item::ItemType>( container.get_attribute<int>( "type_id" ).value_or( -1 ) );
}
