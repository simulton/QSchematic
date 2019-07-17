#pragma once

namespace QSchematic
{
    enum CommandType {
        ItemMoveCommandType      = 0,
        ItemAddCommandType,
        ItemRemoveCommandType,
        ItemVisibilityCommandType,
        NodeResizeCommandType,
        LabelRenameCommandType,
        NodeRotateCommandType,

        QSchematicCommandUserType = 1000
    };
}
