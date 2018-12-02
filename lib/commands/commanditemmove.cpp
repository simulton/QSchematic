#include <memory>
#include "../items/item.h"
#include "commanditemmove.h"

using namespace QSchematic;

CommandItemMove::CommandItemMove(QPointer<Item> item, const QVector2D& moveBy, QUndoCommand* parent) :
    QUndoCommand(parent),
    _item(std::move(item)),
    _moveBy(moveBy)
{
    updateText();
}

int CommandItemMove::id() const
{
    return 0;
}

bool CommandItemMove::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check item
    const CommandItemMove* myCommand = static_cast<const CommandItemMove*>(command);
    if (_item != myCommand->_item) {
        return false;
    }

    // Merge
    _moveBy += myCommand->_moveBy;
    updateText();

    return true;
}

void CommandItemMove::undo()
{
    if (!_item) {
        return;
    }

    _item->moveBy(_moveBy * -1);
}

void CommandItemMove::redo()
{
    if (!_item) {
        return;
    }

    _item->moveBy(_moveBy);
}

void CommandItemMove::updateText()
{
    QString text =
            QStringLiteral("Move item by ( ") +
            QString::number(static_cast<qreal>(_moveBy.x())) +
            QStringLiteral(" / ") +
            QString::number(static_cast<qreal>(_moveBy.y())) +
            QStringLiteral(")");

    setText(text);
}
