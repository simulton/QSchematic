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

        QSchematicCommandUserType = 1000
    };
}
