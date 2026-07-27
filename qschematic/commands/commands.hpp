#pragma once

namespace QSchematic::Commands
{
    enum CommandType {
        ItemMoveCommandType      = 0,
        ItemAddCommandType,
        ItemRemoveCommandType,
        ItemVisibilityCommandType,
        LabelRenameCommandType,
        RectItemResizeCommandType,
        RectItemRotateCommandType,
        WireNetRenameCommandType,
        WirePointMoveCommandType,
        SceneRestoreCommandType,

        QSchematicCommandUserType = 1000
    };
}
