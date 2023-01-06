#include "commands.h"
#include "item_move.h"
#include "../items/item.h"
#include "../items/wire.h"

#include <memory>

using namespace QSchematic;
using namespace QSchematic::Commands;

ItemMove::ItemMove(
    const QVector<std::shared_ptr<Item>>& items,
    const QVector<QVector2D>& moveBy,
    QUndoCommand* parent
) :
    Base(parent),
    _items(items),
    _moveBy(moveBy)
{
    if (_items.count() > 1)
        setText(tr("Move items"));
    else
        setText(tr("Move item"));
}

int
ItemMove::id() const
{
    return ItemMoveCommandType;
}

bool
ItemMove::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id())
        return false;

    // Check items
    const auto myCommand = static_cast<const ItemMove*>(command);
    if (_items.count() != myCommand->_items.count())
        return false;
    if (_items != myCommand->_items)
        return false;

    // Merge
    for (int i = 0; i < _items.count(); i++)
        _moveBy[i] += myCommand->_moveBy[i];

    return true;
}

void
ItemMove::undo()
{
    for (int i = 0; i < _items.count(); i++)
        _items[i]->moveBy(-_moveBy[i]);

    // Simplify the wires
    simplifyWires();
}

void
ItemMove::redo()
{
    for (int i = 0; i < _items.count(); i++)
        _items[i]->moveBy(_moveBy[i]);

    // Simplify the wires
    simplifyWires();
}

void
ItemMove::simplifyWires() const
{
    for (const auto& item : _items) {
        if (auto wire = item->sharedPtr<Wire>())
            wire->simplify();
    }
}
