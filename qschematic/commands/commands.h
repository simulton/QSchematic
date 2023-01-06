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

        QSchematicCommandUserType = 1000
    };
}
