#include "../items/item.h"
#include "../items/wire.h"
#include "../scene.h"
#include "commands.h"
#include "commanditemadd.h"

using namespace QSchematic;

CommandItemAdd::CommandItemAdd(const QPointer<Scene>& scene, const std::shared_ptr<Item>& item, QUndoCommand* parent) :
    QUndoCommand(parent),
    _scene(scene),
    _item(item)
{
    QObject::connect(_scene.data(), &QObject::destroyed, [this]{
        setObsolete(true);
    });
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
    auto wire = dynamic_cast<Wire*>(_item.get());
    if (wire) {
        _scene->removeWire(*wire);
    }

    // Otherwise, fall back to normal item behavior
    else {
        _scene->removeItem(_item.get());
    }
}

void CommandItemAdd::redo()
{
    if (!_scene or !_item) {
        return;
    }

    // Is this a wire?
    auto wire = dynamic_cast<Wire*>(_item.get());
    if (wire) {
        _scene->addWire(wire);
    }

    // Otherwise, fall back to normal item behavior
    else {
        _scene->addItem(_item.get());
    }
}
