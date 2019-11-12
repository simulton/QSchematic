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
        WireNetRenameCommandType,
        WirePointMoveCommandType,

        QSchematicCommandUserType = 1000
    };
}
