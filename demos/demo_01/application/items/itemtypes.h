#pragma once

#include "../../../lib/items/item.h"

enum ItemType {
    OperationType = QSchematic::Item::QSchematicItemUserType + 1,
    OperationConnectorType,
    FancyWireType,
    OperationDemo1Type
};
