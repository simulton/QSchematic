#include "commands.h"
#include "commandrectitemrotate.h"
#include "../items/rectitem.h"

#include <memory>

using namespace QSchematic;

CommandRectItemRotate::CommandRectItemRotate(QPointer<RectItem> item, qreal newAngle, QUndoCommand* parent) :
    UndoCommand(parent),
    _item(item),
    _newAngle(newAngle)
{
    _oldAngle = item->rotation();
    connectDependencyDestroySignal(_item.data());
    setText(tr("RectItem rotate"));
}

int CommandRectItemRotate::id() const
{
    return RectItemRotateCommandType;
}

bool CommandRectItemRotate::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check item
    const CommandRectItemRotate* myCommand = static_cast<const CommandRectItemRotate*>(command);
    if (_item != myCommand->_item) {
        return false;
    }

    // Merge
    _newAngle = myCommand->_newAngle;

    return true;
}

void CommandRectItemRotate::undo()
{
    if (!_item) {
        return;
    }

    _item->setRotation(_oldAngle);
    // Recalculate position
    if (_item->canSnapToGrid()) {
        _item->setPos(_item->itemChange(QGraphicsItem::ItemPositionChange, _item->pos()).toPointF());
    }
}

void CommandRectItemRotate::redo()
{
    if (!_item) {
        return;
    }

    _item->setRotation(_newAngle);
    // Recalculate position
    if (_item->canSnapToGrid()) {
        _item->setPos(_item->itemChange(QGraphicsItem::ItemPositionChange, _item->pos()).toPointF());
    }
}
