#include <memory>
#include "../items/item.h"
#include "commands.h"
#include "commanditemmove.h"

using namespace QSchematic;

CommandItemMove::CommandItemMove(const QVector<std::shared_ptr<Item>>& items, const QVector<QVector2D>& moveBy, QUndoCommand* parent) :
    UndoCommand(parent),
    _items(items),
    _moveBy(moveBy)
{
    if (_items.count() > 1) {
        setText(QStringLiteral("Move items"));
    } else {
        setText(QStringLiteral("Move item"));
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
    for (const auto& item : _items) {
        item->setIsMoving(true);
    }
    for (int i = 0; i < _items.count(); i++) {
        _items[i]->moveBy(-_moveBy[i]);
    }
    for (const auto& item : _items) {
        item->setIsMoving(false);
    }
}

void CommandItemMove::redo()
{
    for (const auto& item : _items) {
        item->setIsMoving(true);
    }
    for (int i = 0; i < _items.count(); i++) {
        _items[i]->moveBy(_moveBy[i]);
    }
    for (const auto& item : _items) {
        item->setIsMoving(false);
    }
}
