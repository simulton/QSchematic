#include <memory>
#include "../items/label.h"
#include "commands.h"
#include "commandlabelrename.h"

using namespace QSchematic;

CommandLabelRename::CommandLabelRename(const QPointer<Label>& label, const QString& newText, QUndoCommand* parent) :
    QUndoCommand(parent),
    _label(label),
    _newText(newText)
{
    _oldText = label->text();

    setText(QStringLiteral("Change label text"));
}

int CommandLabelRename::id() const
{
    return LabelResizeCommandType;
}

bool CommandLabelRename::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check items
    const CommandLabelRename* myCommand = static_cast<const CommandLabelRename*>(command);
    if (_label != myCommand->_label) {
        return false;
    }

    // Merge
    _newText = myCommand->_newText;

    return true;
}

void CommandLabelRename::undo()
{
    if (!_label) {
        return;
    }

    _label->_text = _oldText;
    _label->calculateTextRect();
    _label->update();
}

void CommandLabelRename::redo()
{
    if (!_label) {
        return;
    }

    _label->_text = _newText;
    _label->calculateTextRect();
    _label->update();
}
