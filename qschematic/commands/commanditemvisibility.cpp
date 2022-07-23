#include "commanditemvisibility.h"
#include "commands.h"
#include "../items/item.h"

using namespace QSchematic;

CommandItemVisibility::CommandItemVisibility(const std::shared_ptr<Item>& item, bool newVisibility, QUndoCommand* parent) :
    UndoCommand(parent),
    _item(item),
    _newVisibility(newVisibility)
{
    _oldVisibility = _item->isVisible();
    setText(tr("Change visibility"));
}

int CommandItemVisibility::id() const
{
    return ItemVisibilityCommandType;
}

bool CommandItemVisibility::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id()) {
        return false;
    }

    const CommandItemVisibility* myCommand = dynamic_cast<const CommandItemVisibility*>(command);
    if (!myCommand || _item != myCommand->_item) {
        return false;
    }

    _newVisibility = myCommand->_newVisibility;

    return true;
}

void CommandItemVisibility::undo()
{
    if (!_item) {
        return;
    }

    _item->setVisible(_oldVisibility);
}

void CommandItemVisibility::redo()
{
    if (!_item) {
        return;
    }

    _item->setVisible(_newVisibility);
}
