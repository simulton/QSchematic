#include "../../../lib/items/node.h"
#include "../../../lib/items/label.h"
#include "commands.h"
#include "commandnoderename.h"

CommandNodeRename::CommandNodeRename(const QPointer<QSchematic::Node>& node, const QString& newText, QUndoCommand* parent) :
    QUndoCommand(parent),
    _node(node),
    _newText(newText)
{
    _oldText = node->label()->text();

    QObject::connect(_node.data(), &QObject::destroyed, [this]{
        setObsolete(true);
    });
    setText(QStringLiteral("Rename node"));
}

int CommandNodeRename::id() const
{
    return NodeRenameCommandType;
}

bool CommandNodeRename::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id()) {
        return false;
    }

    const CommandNodeRename* myCommand = dynamic_cast<const CommandNodeRename*>(command);
    if (!myCommand) {
        return false;
    }

    _newText = myCommand->_newText;

    return true;
}

void CommandNodeRename::undo()
{
    if (!_node) {
        return;
    }

    _node->label()->setText(_oldText);
}

void CommandNodeRename::redo()
{
    if (!_node) {
        return;
    }

    _node->label()->setText(_newText);
}
