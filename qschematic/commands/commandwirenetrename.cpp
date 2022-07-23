#include "commands.h"
#include "commandwirenetrename.h"
#include "../items/label.h"
#include "../items/wire.h"

using namespace QSchematic;

CommandWirenetRename::CommandWirenetRename(const std::shared_ptr<WireNet>& net, const QString& newText, QUndoCommand* parent) :
    UndoCommand(parent),
    _newText(newText)
{
    _net = net;
    _oldText = net->name();
    setText(tr("Rename wirenet"));
}

int CommandWirenetRename::id() const
{
    return WireNetRenameCommandType;
}

bool CommandWirenetRename::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id()) {
        return false;
    }

    auto myCommand = dynamic_cast<const CommandWirenetRename*>(command);
    if (!myCommand || _net != myCommand->_net) {
        return false;
    }

    _newText = myCommand->_newText;

    return true;
}

void CommandWirenetRename::undo()
{
    if (!_net) {
        return;
    }

    _net->set_name(_oldText);
}

void CommandWirenetRename::redo()
{
    if (!_net) {
        return;
    }

    _net->set_name(_newText);
}
