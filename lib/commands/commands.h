#pragma once

namespace QSchematic
{
    enum CommandType {
        ItemMoveCommandType      = 0,
        ItemAddCommandType,
        ItemRemoveCommandType,
        NodeResizeCommandType,

        QSchematicCommandUserType = 1000
    };
}
