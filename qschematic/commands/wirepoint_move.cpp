#include "wirepoint_move.hpp"
#include "commands.hpp"
#include "../items/wire.hpp"
#include "../wire_system/manager.hpp"

using namespace QSchematic;
using namespace QSchematic::Commands;

WirepointMove::WirepointMove(
    std::shared_ptr<Items::Wire> wire,
    int index,
    const QPointF& pos,
    QUndoCommand* parent
) :
    Base(parent),
    _wire{std::move(wire)}
{
    _old.pointIndex = index;
    _old.pos = _wire->pointsAbsolute().at(index);
    _new.pointIndex = index;
    _new.pos = pos;

    // Nothing to do if the new position is the same
    if (_old.pos == pos)
        setObsolete(true);

    setText(tr("Move wire point"));
}

int
WirepointMove::id() const
{
    return WirePointMoveCommandType;
}

bool
WirepointMove::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id())
        return false;

    // Check items
    const auto myCommand = static_cast<const WirepointMove*>(command);
    if (_wire != myCommand->_wire)
        return false;

    _new = myCommand->_new;

    if (_old.pos == _new.pos)
        setObsolete(true);

    return true;
}

void
WirepointMove::undo()
{
    // Sanity check
    if (!_wire) [[unlikely]]
        return;

    // Get wire manager
    wire_system::manager* wm = _wire->manager();
    if (!wm) [[unlikely]]
        return;

    // Move the wire point
    _wire->move_point_to(_old.pointIndex, _old.pos);

    // Let the wiresystem manager know
    // ToDo: The wire should probably do this internally
    wm->point_moved_by_user(*_wire, _old.pointIndex);
}

void
WirepointMove::redo()
{
    // Sanity check
    if (!_wire) [[unlikely]]
        return;

    // Get wire manager
    wire_system::manager* wm = _wire->manager();
    if (!wm) [[unlikely]]
        return;

    // Move the wire point
    _wire->move_point_to(_new.pointIndex, _new.pos);

    // Let the wiresystem manager know
    // ToDo: The wire should probably do this internally
    wm->point_moved_by_user(*_wire, _new.pointIndex);
}
