#include "../qschematic/items/itemfactory.h"
#include "itemtypes.h"
#include "operation.h"
#include "operationconnector.h"
#include "operationdemo1.h"
#include "customitemfactory.h"
#include "fancywire.h"
#include "flowstart.h"
#include "flowend.h"

QSchematic::OriginMgrT<QSchematic::Item> CustomItemFactory::fromContainer(const Gpds::Container& container)
{
    // Extract the type
    QSchematic::Item::ItemType type = QSchematic::ItemFactory::extractType(container);

    switch (static_cast<ItemType>(type)) {
    case ItemType::OperationType:
        return QSchematic::make_origin<Operation>();

    case ItemType::OperationConnectorType:
        return QSchematic::make_origin<OperationConnector>();

    case ItemType::OperationDemo1Type:
        return QSchematic::make_origin<OperationDemo1>();

    case ItemType::FancyWireType:
        return QSchematic::make_origin<FancyWire>();

    case ItemType::FlowStartType:
        return QSchematic::make_origin<FlowStart>();

    case ItemType::FlowEndType:
        return QSchematic::make_origin<FlowEnd>();
    }

    return {};
}
