#pragma once

#include <qschematic/items/item.h>

enum ItemType {
    OperationType = QSchematic::Item::QSchematicItemUserType + 1,
    OperationConnectorType,
    FancyWireType,
    OperationDemo1Type,
    FlowStartType,
    FlowEndType
};
