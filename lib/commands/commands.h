#pragma once

namespace QSchematic
{
    enum CommandType {
        ItemMoveCommandType      = 0,
        NodeResizeCommandType,
        LabelResizeCommandType,
        ItemSetVisibleCommand,

        QSchematicCommandUserType = 1000
    };
}
