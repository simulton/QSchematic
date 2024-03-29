#include "commands.hpp"
#include "wirenet_rename.hpp"
#include "../items/label.hpp"
#include "../items/wire.hpp"
#include "../items/wirenet.hpp"

using namespace QSchematic;
using namespace QSchematic::Commands;

WirenetRename::WirenetRename(const std::shared_ptr<Items::WireNet>& net, const QString& newText, QUndoCommand* parent) :
    Base(parent),
    _newText(newText)
{
    _net = net;
    _oldText = net->name();
    setText(tr("Rename wirenet"));
}

int
WirenetRename::id() const
{
    return WireNetRenameCommandType;
}

bool
WirenetRename::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id())
        return false;

    auto myCommand = dynamic_cast<const WirenetRename*>(command);
    if (!myCommand || _net != myCommand->_net)
        return false;

    _newText = myCommand->_newText;

    return true;
}

void
WirenetRename::undo()
{
    if (!_net)
        return;

    _net->set_name(_oldText);
}

void
WirenetRename::redo()
{
    if (!_net)
        return;

    _net->set_name(_newText);
}
