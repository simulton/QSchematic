#include <QJsonObject>

#include "../../../lib/items/item.h"
#include "../../../lib/items/itemfactory.h"
#include "itemtypes.h"
#include "condition.h"
#include "operation.h"
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

    case ItemType::OperationType:
        item.reset(new Operation);
        break;
    }

    // Sanity check
    if (!item) {
        return nullptr;
    }

    // Load
    item->fromJson(object);

    return item;
}
