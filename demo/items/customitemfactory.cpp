#include "itemtypes.h"
#include "operation.h"
#include "operationconnector.h"
#include "operationdemo1.h"
#include "customitemfactory.h"
#include "fancywire.h"
#include "flowstart.h"
#include "flowend.h"

#include <qschematic/items/itemfactory.h>

std::shared_ptr<QSchematic::Item> CustomItemFactory::from_container(const gpds::container& container)
{
    // Extract the type
    QSchematic::Item::ItemType type = QSchematic::ItemFactory::extractType(container);

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
    }

    return {};
}
