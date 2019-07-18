#include "../qschematic/items/item.h"
#include "../qschematic/items/itemfactory.h"
#include "itemtypes.h"
#include "operation.h"
#include "operationconnector.h"
#include "operationdemo1.h"
#include "customitemfactory.h"
#include "fancywire.h"
#include "flowstart.h"
#include "flowend.h"

std::unique_ptr<QSchematic::Item> CustomItemFactory::fromContainer(const Gpds::Container& container)
{
    // Extract the type
    QSchematic::Item::ItemType type = QSchematic::ItemFactory::extractType(container);

    // Create the item
    std::unique_ptr<QSchematic::Item> item;
    switch (static_cast<ItemType>(type)) {
    case ItemType::OperationType:
        item.reset(new Operation);
        break;

    case ItemType::OperationConnectorType:
        item.reset(new OperationConnector);
        break;

    case ItemType::OperationDemo1Type:
        item.reset(new OperationDemo1);
        break;

    case ItemType::FancyWireType:
        item.reset(new FancyWire);
        break;

    case ItemType::FlowStartType:
        item.reset(new FlowStart);
        break;

    case ItemType::FlowEndType:
        item.reset(new FlowEnd);
        break;
    }

    return item;
}
