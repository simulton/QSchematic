#include "commands.h"
#include "commanditemmove.h"
#include "../items/item.h"
#include "../items/wire.h"

#include <memory>

using namespace QSchematic;

CommandItemMove::CommandItemMove(const QVector<std::shared_ptr<Item>>& items, const QVector<QVector2D>& moveBy, QUndoCommand* parent) :
    UndoCommand(parent),
    _items(items),
    _moveBy(moveBy)
{
    if (_items.count() > 1) {
        setText(tr("Move items"));
    } else {
        setText(tr("Move item"));
    }
}

int CommandItemMove::id() const
{
    return ItemMoveCommandType;
}

bool CommandItemMove::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check items
    const auto myCommand = static_cast<const CommandItemMove*>(command);
    if (_items.count() != myCommand->_items.count()) {
        return false;
    }
    if (_items != myCommand->_items) {
        return false;
    }

    // Merge
    for (int i = 0; i < _items.count(); i++) {
        _moveBy[i] += myCommand->_moveBy[i];
    }

    return true;
}

void CommandItemMove::undo()
{
    for (int i = 0; i < _items.count(); i++) {
        _items[i]->moveBy(-_moveBy[i]);
    }
    // Simplify the wires
    simplifyWires();
}

void CommandItemMove::redo()
{
    for (int i = 0; i < _items.count(); i++) {
        _items[i]->moveBy(_moveBy[i]);
    }
    // Simplify the wires
    simplifyWires();
}

void CommandItemMove::simplifyWires() const
{
    for (const auto& item : _items) {
        if (auto wire = item->sharedPtr<Wire>()) {
            wire->simplify();
        }
    }
}
