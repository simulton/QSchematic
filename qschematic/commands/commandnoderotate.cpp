#include <memory>
#include "../items/node.h"
#include "commands.h"
#include "commandnoderotate.h"

using namespace QSchematic;

CommandNodeRotate::CommandNodeRotate(QPointer<Node> node, qreal newAngle, QUndoCommand* parent) :
    UndoCommand(parent),
    _node(node),
    _newAngle(newAngle)
{
    _oldAngle = node->rotation();
    connectDependencyDestroySignal(_node.data());
    setText(tr("Node rotate"));
}

int CommandNodeRotate::id() const
{
    return NodeRotateCommandType;
}

bool CommandNodeRotate::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check item
    const CommandNodeRotate* myCommand = static_cast<const CommandNodeRotate*>(command);
    if (_node != myCommand->_node) {
        return false;
    }

    // Merge
    _newAngle = myCommand->_newAngle;

    return true;
}

void CommandNodeRotate::undo()
{
    if (!_node) {
        return;
    }

    _node->setRotation(_oldAngle);
    // Recalculate position
    if (_node->canSnapToGrid()) {
        _node->setPos(_node->itemChange(QGraphicsItem::ItemPositionChange, _node->pos()).toPointF());
    }
}

void CommandNodeRotate::redo()
{
    if (!_node) {
        return;
    }

    _node->setRotation(_newAngle);
    // Recalculate position
    if (_node->canSnapToGrid()) {
        _node->setPos(_node->itemChange(QGraphicsItem::ItemPositionChange, _node->pos()).toPointF());
    }
}
