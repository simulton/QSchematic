#pragma once

#include <qschematic/items/item.hpp>

enum ItemType {
    OperationType = QSchematic::Items::Item::QSchematicItemUserType + 1,
    OperationConnectorType,
    FancyWireType,
    OperationDemo1Type,
    FlowStartType,
    FlowEndType,
    WidgetDial,
    WidgetTextedit,
};
