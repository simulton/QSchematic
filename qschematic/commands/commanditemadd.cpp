#include "../items/item.h"
#include "../items/wire.h"
#include "../scene.h"
#include "commands.h"
#include "commanditemadd.h"

using namespace QSchematic;

CommandItemAdd::CommandItemAdd(const QPointer<Scene>& scene, const std::shared_ptr<Item>& item, QUndoCommand* parent) :
    UndoCommand(parent),
    _scene(scene),
    _item(item)
{
    connectDependencyDestroySignal(_scene.data());
    setText(QStringLiteral("Add item"));
}

int CommandItemAdd::id() const
{
    return ItemAddCommandType;
}

bool CommandItemAdd::mergeWith(const QUndoCommand* command)
{
    Q_UNUSED(command)

    return false;
}

void CommandItemAdd::undo()
{
    if (!_scene or !_item) {
        return;
    }

    // Is this a wire?
    auto wire = std::dynamic_pointer_cast<Wire>(_item);
    if (wire) {
        _scene->removeWire(wire);
    }

    // Otherwise, fall back to normal item behavior
    else {
        _scene->removeItem(_item);
    }
}

void CommandItemAdd::redo()
{
    if (!_scene or !_item) {
        return;
    }

    // Is this a wire?
    auto wire = std::dynamic_pointer_cast<Wire>(_item);
    if (wire) {
        if (wire->net()) {
            if (not _scene->nets().contains(wire->net())) {
                _scene->addWireNet(wire->net());
            }
            wire->net()->addWire(wire);
            _scene->addItem(wire);
        } else {
            _scene->addWire(wire);
        }
        for (int i = 0; i < wire->wirePointsRelative().count(); i++) {
            wire->net()->pointMovedByUser(*wire.get(), i);
        }
    }

    // Otherwise, fall back to normal item behavior
    else {
        _scene->addItem(_item);
    }
}
