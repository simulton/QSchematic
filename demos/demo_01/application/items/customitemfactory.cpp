#include <QJsonObject>

#include "../../../lib/items/item.h"
#include "../../../lib/items/itemfactory.h"
#include "itemtypes.h"
#include "condition.h"
#include "conditionconnector.h"
#include "operation.h"
#include "operationconnector.h"
#include "customitemfactory.h"

std::unique_ptr<QSchematic::Item> CustomItemFactory::fromJson(const QJsonObject& object)
{
    // Extract the type
    QSchematic::Item::ItemType type = QSchematic::ItemFactory::extractType(object);

    // Create the item
    std::unique_ptr<QSchematic::Item> item;
    switch (static_cast<ItemType>(type)) {
    case ItemType::ConditionType:
        item.reset(new Condition);
        break;

    case ItemType::ConditionConnectorType:
        item.reset(new ConditionConnector);
        break;

    case ItemType::OperationType:
        item.reset(new Operation);
        break;

    case ItemType::OperationConnectorType:
        item.reset(new OperationConnector);
        break;
    }

    return item;
}
