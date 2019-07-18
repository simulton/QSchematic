#include <memory>
#include "../items/item.h"
#include "commands.h"
#include "commanditemmove.h"

using namespace QSchematic;

CommandItemMove::CommandItemMove(const QVector<std::shared_ptr<Item>>& items, const QVector2D& moveBy, QUndoCommand* parent) :
    QUndoCommand(parent),
    _items(items),
    _moveBy(moveBy)
{
    setText(QStringLiteral("Move item"));
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
    const CommandItemMove* myCommand = static_cast<const CommandItemMove*>(command);
    if (_items.count() != myCommand->_items.count()) {
        return false;
    }
    for (int i = 0; i < _items.count(); i++) {
        if (_items.at(i) != myCommand->_items.at(i)) {
            return false;
        }
    }

    // Merge
    _moveBy += myCommand->_moveBy;

    return true;
}

void CommandItemMove::undo()
{
    for (auto item : _items) {
        if (!item) {
            continue;
        }

        item->moveBy(_moveBy * -1);
    }
}

void CommandItemMove::redo()
{
    for (auto item : _items) {
        if (!item) {
            continue;
        }

        item->moveBy(_moveBy);
    }
}
