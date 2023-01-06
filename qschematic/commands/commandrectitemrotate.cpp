#include "commands.h"
#include "commandrectitemrotate.h"
#include "../items/rectitem.h"

#include <memory>

using namespace QSchematic;
using namespace QSchematic::Commands;

RectItemRotate::RectItemRotate(QPointer<RectItem> item, qreal newAngle, QUndoCommand* parent) :
    Base(parent),
    _item(item),
    _newAngle(newAngle)
{
    _oldAngle = item->rotation();
    connectDependencyDestroySignal(_item.data());
    setText(tr("RectItem rotate"));
}

int
RectItemRotate::id() const
{
    return RectItemRotateCommandType;
}

bool
RectItemRotate::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id())
        return false;

    // Check item
    const RectItemRotate* myCommand = static_cast<const RectItemRotate*>(command);
    if (_item != myCommand->_item)
        return false;

    // Merge
    _newAngle = myCommand->_newAngle;

    return true;
}

void
RectItemRotate::undo()
{
    if (!_item)
        return;

    _item->setRotation(_oldAngle);

    // Recalculate position
    if (_item->canSnapToGrid())
        _item->setPos(_item->itemChange(QGraphicsItem::ItemPositionChange, _item->pos()).toPointF());
}

void
RectItemRotate::redo()
{
    if (!_item)
        return;

    _item->setRotation(_newAngle);

    // Recalculate position
    if (_item->canSnapToGrid())
        _item->setPos(_item->itemChange(QGraphicsItem::ItemPositionChange, _item->pos()).toPointF());
}
