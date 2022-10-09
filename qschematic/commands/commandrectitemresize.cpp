#include "../items/rectitem.h"
#include "commands.h"
#include "commandrectitemresize.h"

using namespace QSchematic;

CommandRectItemResize::CommandRectItemResize(QPointer<RectItem> item, const QPointF& newPos, const QSizeF& newSize, QUndoCommand* parent) :
    UndoCommand(parent),
    _item(item),
    _newPos(newPos),
    _newSize(newSize)
{
    _oldPos = item->pos();
    _oldSize = item->size();
    connectDependencyDestroySignal(_item.data());
    setText(tr("RectItem resize"));
}

int CommandRectItemResize::id() const
{
    return RectItemResizeCommandType;
}

bool CommandRectItemResize::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check item
    const CommandRectItemResize* myCommand = static_cast<const CommandRectItemResize*>(command);
    if (_item != myCommand->_item) {
        return false;
    }

    // Merge
    _newPos = myCommand->_newPos;
    _newSize = myCommand->_newSize;

    return true;
}

void CommandRectItemResize::undo()
{
    if (!_item) {
        return;
    }

    _item->setSize(_oldSize);
    _item->setPos(_oldPos);
}

void CommandRectItemResize::redo()
{
    if (!_item) {
        return;
    }

    _item->setSize(_newSize);
    _item->setPos(_newPos);
}
