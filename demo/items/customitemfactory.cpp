#include "itemtypes.hpp"
#include "operation.hpp"
#include "operationconnector.hpp"
#include "operationdemo1.hpp"
#include "customitemfactory.hpp"
#include "fancywire.hpp"
#include "flowstart.hpp"
#include "flowend.hpp"
#include "widgets/dial.hpp"
#include "widgets/textedit.hpp"

#include <qschematic/items/itemfactory.hpp>

std::shared_ptr<QSchematic::Items::Item> CustomItemFactory::from_container(const gpds::container& container)
{
    // Extract the type
    QSchematic::Items::Item::ItemType type = QSchematic::Items::Factory::extractType(container);

    switch (static_cast<ItemType>(type)) {
    case ItemType::OperationType:
        return std::make_shared<Operation>();

    case ItemType::OperationConnectorType:
        return std::make_shared<OperationConnector>();

    case ItemType::OperationDemo1Type:
        return std::make_shared<OperationDemo1>();

    case ItemType::FancyWireType:
        return std::make_shared<FancyWire>();

    case ItemType::FlowStartType:
        return std::make_shared<FlowStart>();

    case ItemType::FlowEndType:
        return std::make_shared<FlowEnd>();

    case ItemType::WidgetDial:
        return std::make_shared<Items::Widgets::Dial>();

    case ItemType::WidgetTextedit:
        return std::make_shared<Items::Widgets::Textedit>();
    }

    return {};
}
