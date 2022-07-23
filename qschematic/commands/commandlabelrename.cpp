#include "commands.h"
#include "commandlabelrename.h"
#include "../items/label.h"

using namespace QSchematic;

CommandLabelRename::CommandLabelRename(const QPointer<Label>& label, const QString& newText, QUndoCommand* parent) :
    UndoCommand(parent),
    _label(label),
    _newText(newText)
{
    _oldText = _label->text();
    // TODO: is there a reason for no destruction tracking here? (leaves it alone for now / Oscar)
    setText(tr("Rename label"));
}

int CommandLabelRename::id() const
{
    return LabelRenameCommandType;
}

bool CommandLabelRename::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id()) {
        return false;
    }

    const CommandLabelRename* myCommand = dynamic_cast<const CommandLabelRename*>(command);
    if (!myCommand || _label != myCommand->_label) {
        return false;
    }

    _newText = myCommand->_newText;

    return true;
}

void CommandLabelRename::undo()
{
    if (!_label) {
        return;
    }

    _label->setText(_oldText);
    _label->update();
}

void CommandLabelRename::redo()
{
    if (!_label) {
        return;
    }

    _label->setText(_newText);
    _label->update();
}
