#pragma once

#include "../../../lib/commands/commands.h"

enum CommandType {
    NodeAddConnectorCommandType = QSchematic::QSchematicCommandUserType + 1,
    LabelRenameCommandType
};
