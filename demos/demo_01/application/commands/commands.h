#pragma once

#include "../../../lib/commands/commands.h"

enum CommandType {
    ItemVisibilityCommandType = QSchematic::QSchematicCommandUserType + 1,
    NodeAddConnectorCommandType,
    LabelRenameCommandType
};
