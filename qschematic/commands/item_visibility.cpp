#include "item_visibility.h"
#include "commands.h"
#include "../items/item.h"

using namespace QSchematic;
using namespace QSchematic::Commands;

ItemVisibility::ItemVisibility(const std::shared_ptr<Items::Item>& item, bool newVisibility, QUndoCommand* parent) :
    Base(parent),
    _item(item),
    _newVisibility(newVisibility)
{
    _oldVisibility = _item->isVisible();
    setText(tr("Change visibility"));
}

int
ItemVisibility::id() const
{
    return ItemVisibilityCommandType;
}

bool
ItemVisibility::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id())
        return false;

    const ItemVisibility* myCommand = dynamic_cast<const ItemVisibility*>(command);
    if (!myCommand || _item != myCommand->_item)
        return false;

    _newVisibility = myCommand->_newVisibility;

    return true;
}

void
ItemVisibility::undo()
{
    if (!_item)
        return;

    _item->setVisible(_oldVisibility);
}

void
ItemVisibility::redo()
{
    if (!_item)
        return;

    _item->setVisible(_newVisibility);
}
