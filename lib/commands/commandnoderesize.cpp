#include <memory>
#include "../items/node.h"
#include "commands.h"
#include "commandnoderesize.h"

using namespace QSchematic;

CommandNodeResize::CommandNodeResize(QPointer<Node> node, const QPointF& newPos, const QSizeF& newSize, QUndoCommand* parent) :
    QUndoCommand(parent),
    _node(node),
    _newPos(newPos),
    _newSize(newSize)
{
    _oldPos = node->pos();
    _oldSize = node->size();
    QObject::connect(_node.data(), &QObject::destroyed, [this]{
        setObsolete(true);
    });
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
    _newPos = myCommand->_newPos;
    _newSize = myCommand->_newSize;

    return true;
}

void CommandNodeResize::undo()
{
    if (!_node) {
        return;
    }

    _node->setSize(_oldSize);
    _node->setPos(_oldPos);
}

void CommandNodeResize::redo()
{
    if (!_node) {
        return;
    }

    _node->setSize(_newSize);
    _node->setPos(_newPos);
}
