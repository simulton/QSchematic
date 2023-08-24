#include "commands.hpp"
#include "item_move.hpp"
#include "../items/item.hpp"
#include "../items/wire.hpp"

#include <memory>

using namespace QSchematic;
using namespace QSchematic::Commands;

ItemMove::ItemMove(
    const QVector<std::shared_ptr<Items::Item>>& items,
    QVector2D moveBy,
    QUndoCommand* parent
) :
    Base(parent),
    _items{ items },
    _moveBy{ std::move(moveBy) }
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
    if (_items != myCommand->_items)
        return false;

    // Merge
    _moveBy += myCommand->_moveBy;

    return true;
}

void
ItemMove::undo()
{
    for (auto& item : _items)
        item->moveBy(-_moveBy);

    simplifyWires();
}

void
ItemMove::redo()
{
    for (auto& item : _items)
        item->moveBy(_moveBy);

    simplifyWires();
}

void
ItemMove::simplifyWires() const
{
    for (const auto& item : _items) {
        if (auto wire = item->sharedPtr<Items::Wire>())
            wire->simplify();
    }
}
