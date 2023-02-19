#include "commands.h"
#include "label_rename.h"
#include "../items/label.h"

using namespace QSchematic;
using namespace QSchematic::Commands;

LabelRename::LabelRename(const QPointer<Items::Label>& label, const QString& newText, QUndoCommand* parent) :
    Base(parent),
    _label(label),
    _newText(newText)
{
    _oldText = _label->text();
    // TODO: is there a reason for no destruction tracking here? (leaves it alone for now / Oscar)
    setText(tr("Rename label"));
}

int
LabelRename::id() const
{
    return LabelRenameCommandType;
}

bool
LabelRename::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id())
        return false;

    const LabelRename* myCommand = dynamic_cast<const LabelRename*>(command);
    if (!myCommand || _label != myCommand->_label)
        return false;

    _newText = myCommand->_newText;

    return true;
}

void
LabelRename::undo()
{
    if (!_label)
        return;

    _label->setText(_oldText);
    _label->update();
}

void
LabelRename::redo()
{
    if (!_label)
        return;

    _label->setText(_newText);
    _label->update();
}
