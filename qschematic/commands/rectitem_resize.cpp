#include "../items/rectitem.hpp"
#include "commands.hpp"
#include "rectitem_resize.hpp"

using namespace QSchematic;
using namespace QSchematic::Commands;

RectItemResize::RectItemResize(QPointer<Items::RectItem> item, const QPointF& newPos, const QSizeF& newSize, QUndoCommand* parent) :
    Base(parent),
    _item(item),
    _newPos(newPos),
    _newSize(newSize)
{
    _oldPos = item->pos();
    _oldSize = item->size();
    connectDependencyDestroySignal(_item.data());
    setText(tr("RectItem resize"));
}

int
RectItemResize::id() const
{
    return RectItemResizeCommandType;
}

bool
RectItemResize::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id())
        return false;

    // Check item
    auto myCommand = dynamic_cast<const RectItemResize*>(command);
    if (_item != myCommand->_item)
        return false;

    // Merge
    _newPos = myCommand->_newPos;
    _newSize = myCommand->_newSize;

    return true;
}

void
RectItemResize::undo()
{
    if (!_item)
        return;

    _item->setSize(_oldSize);
    _item->setPos(_oldPos);
}

void
RectItemResize::redo()
{
    if (!_item)
        return;

    _item->setSize(_newSize);
    _item->setPos(_newPos);
}
