#include <memory>
#include "../items/item.h"
#include "commands.h"
#include "commanditemsetvisible.h"

using namespace QSchematic;

CommandItemSetVisible::CommandItemSetVisible(const QPointer<Item>& item, bool visible, QUndoCommand* parent) :
    QUndoCommand(parent),
    _item(item),
    _newVisibility(visible)
{
    QObject::connect(_item.data(), &QObject::destroyed, [this]{
        setObsolete(true);
    });
    _oldVisibility = item->isVisible();

    setText(QStringLiteral("Change visibility"));
}

int CommandItemSetVisible::id() const
{
    return ItemSetVisibleCommand;
}

bool CommandItemSetVisible::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check items
    const CommandItemSetVisible* myCommand = static_cast<const CommandItemSetVisible*>(command);
    if (_item != myCommand->_item) {
        return false;
    }

    // Merge
    _newVisibility = myCommand->_newVisibility;

    return true;
}

void CommandItemSetVisible::undo()
{
    if (!_item) {
        return;
    }

    _item->QGraphicsObject::setVisible(_oldVisibility);
}

void CommandItemSetVisible::redo()
{
    if (!_item) {
        return;
    }

    _item->QGraphicsObject::setVisible(_newVisibility);
}
