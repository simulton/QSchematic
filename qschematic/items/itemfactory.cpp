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

void ItemFactory::setCustomItemsFactory(const std::function<std::shared_ptr<Item>(const gpds::container&)>& factory)
{
    _customItemFactory = factory;
}

std::shared_ptr<Item> ItemFactory::from_container(const gpds::container& container) const
{
    // First, try custom types
    if (_customItemFactory) {
        if (auto item = _customItemFactory(container)) {
            return item;
        }
    }

    // Extract the type
    auto type = ItemFactory::extractType(container);

    // Fall back to internal types
    switch (type) {
    case Item::NodeType:
        return QSchematic::mk_sh<Node>();

    case Item::WireType:
        return QSchematic::mk_sh<Wire>();

    case Item::WireRoundedCornersType:
        return QSchematic::mk_sh<WireRoundedCorners>();

    case Item::SplineWireType:
        return QSchematic::mk_sh<SplineWire>();

    case Item::ConnectorType:
        return QSchematic::mk_sh<Connector>();

    case Item::LabelType:
        return QSchematic::mk_sh<Label>();

    case Item::QSchematicItemUserType:
        break;
    }

    return {};
}

Item::ItemType ItemFactory::extractType(const gpds::container& container)
{
    return static_cast<Item::ItemType>( container.get_attribute<int>( "type_id" ).value_or( -1 ) );
}
