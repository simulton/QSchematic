#include <memory>
#include "../items/node.h"
#include "commands.h"
#include "commandnoderesize.h"

using namespace QSchematic;

CommandNodeResize::CommandNodeResize(QPointer<Node> node, const QPoint& newGridPos, const QSize& newSize, QUndoCommand* parent) :
    QUndoCommand(parent),
    _node(node),
    _newGridPos(newGridPos),
    _newSize(newSize)
{
    _oldGridPos = node->gridPos();
    _oldSize = node->size();
    setText(QStringLiteral("Node resize"));
}

int CommandNodeResize::id() const
{
    return NodeResizeCommandType;
}

bool CommandNodeResize::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check item
    const CommandNodeResize* myCommand = static_cast<const CommandNodeResize*>(command);
    if (_node != myCommand->_node) {
        return false;
    }

    // Merge
    _newGridPos = myCommand->_newGridPos;
    _newSize = myCommand->_newSize;

    return true;
}

void CommandNodeResize::undo()
{
    if (!_node) {
        return;
    }

    _node->setGridPos(_oldGridPos);
    _node->setSize(_oldSize);
}

void CommandNodeResize::redo()
{
    if (!_node) {
        return;
    }

    _node->setGridPos(_newGridPos);
    _node->setSize(_newSize);
}
