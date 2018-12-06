#include <QJsonObject>

#include "../../../lib/items/item.h"
#include "../../../lib/items/itemfactory.h"
#include "itemtypes.h"
#include "operation.h"
#include "operationconnector.h"
#include "customitemfactory.h"
#include "fancywire.h"

std::unique_ptr<QSchematic::Item> CustomItemFactory::fromJson(const QJsonObject& object)
{
    // Extract the type
    QSchematic::Item::ItemType type = QSchematic::ItemFactory::extractType(object);

    // Create the item
    std::unique_ptr<QSchematic::Item> item;
    switch (static_cast<ItemType>(type)) {
    case ItemType::OperationType:
        item.reset(new Operation);
        break;

    case ItemType::OperationConnectorType:
        item.reset(new OperationConnector);
        break;

    case ItemType::FancyWireType:
        item.reset(new FancyWire);
        break;
    }

    return item;
}
