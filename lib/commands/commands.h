#pragma once

namespace QSchematic
{
    enum CommandType {
        ItemMoveCommandType      = 0,
        ItemAddCommandType,
        NodeResizeCommandType,

        QSchematicCommandUserType = 1000
    };
}
