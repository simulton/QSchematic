#include "commands.h"
#include "commandwirepointmove.h"
#include "../scene.h"
#include "../items/item.h"

using namespace QSchematic;

CommandWirepointMove::CommandWirepointMove(Scene* scene, const std::shared_ptr<Wire>& wire,
                                           int index,
                                           const QPointF& pos, QUndoCommand* parent) :
        _scene(scene),
        UndoCommand(parent),
        _wire(wire)
{
    _oldPos = _wire->pointsAbsolute();
    if (_oldPos[index] == pos) {
        setObsolete(true);
    }
    _newPos = _wire->pointsAbsolute();
    _newPos[index] = pos;
    _oldNet = _wire->net();
    setText(tr("Move wire point"));
}

int CommandWirepointMove::id() const
{
    return WirePointMoveCommandType;
}

bool CommandWirepointMove::mergeWith(const QUndoCommand* command)
{
    // Check id
    if (id() != command->id()) {
        return false;
    }

    // Check items
    const auto myCommand = static_cast<const CommandWirepointMove*>(command);
    if (_wire != myCommand->_wire) {
        return false;
    }

    _newPos = myCommand->_newPos;
    _newNet = myCommand->_newNet;

    if (_oldPos == _newPos) {
        setObsolete(true);
    }

    return true;
}

void CommandWirepointMove::undo()
{
    _newNet = _wire->net();
    // The wire might get simplified after this action is executed. In most
    // cases the wire should be back to the state it was before this command
    // but there are cases were we can't rely on the other commands so we need
    // to make sure that we have the correct amount of points.
    if (_oldPos.count() != _wire->wirePointsRelative().count()) {
        int diff = _oldPos.count() - _wire->wirePointsRelative().count();
        if (diff > 0) {
            for (int i = 0; i < diff; i++) {
                _wire->append_point(QPointF());
            }
        } else {
            for (int i = 0; i < qAbs(diff); i++) {
                _wire->removeLastPoint();
            }
        }
    }

    for (int i = 0; i < _oldPos.count(); i++) {
        if (_newPos[i] != _oldPos[i]) {
            _wire->move_point_to(i, _oldPos[i]);
            _scene->wire_manager()->point_moved_by_user(*_wire.get(), i);
        }
    }
    if (_oldNet != _wire->net()) {
        auto tmpNet = _wire->net();
        for (const auto& wire : tmpNet->wires()) {
            _oldNet->addWire(wire);
            tmpNet->removeWire(wire);
        }
        // If not already in the scene add the existing net
        if (!_scene->wire_manager()->nets().contains(_oldNet)) {
            _scene->wire_manager()->add_net(_oldNet);
        }
        // Remove the tmp net
        _scene->wire_manager()->remove_net(tmpNet);
    }
}

void CommandWirepointMove::redo()
{
    for (int i = 0; i < _newPos.count(); i++) {
        if (_newPos[i] != _oldPos[i]) {
            _wire->move_point_to(i, _newPos[i]);
            _scene->wire_manager()->point_moved_by_user(*_wire.get(), i);
        }
    }
    // Use existing net
    if (_newNet && _newNet != _wire->net()) {
        // Retrieve temporary net
        auto tmpNet = _wire->net();
        // Move all wires to the existing net
        for (const auto& wire : tmpNet->wires()) {
            _newNet->addWire(wire);
            tmpNet->removeWire(wire);
        }
        // If not already in the scene add the existing net
        if (!_scene->wire_manager()->nets().contains(_newNet)) {
            _scene->wire_manager()->add_net(_newNet);
        }
        // Remove the tmp net
        _scene->wire_manager()->remove_net(tmpNet);
    } else {
        _newNet = _wire->net();
    }
}
