#include "../items/item.h"
#include "../items/wire.h"
#include "../scene.h"
#include "commands.h"
#include "commanditemremove.h"

using namespace QSchematic;

CommandItemRemove::CommandItemRemove(const QPointer<Scene>& scene, const std::shared_ptr<Item>& item, QUndoCommand* parent) :
    QUndoCommand(parent),
    _scene(scene),
    _item(item)
{
    QObject::connect(_scene.data(), &QObject::destroyed, [this]{
        setObsolete(true);
    });

    setText(QStringLiteral("Remove item"));
}

int CommandItemRemove::id() const
{
    return ItemRemoveCommandType;
}

bool CommandItemRemove::mergeWith(const QUndoCommand* command)
{
    Q_UNUSED(command)

    return false;
}

void CommandItemRemove::undo()
{
    if (!_scene or !_item) {
        return;
    }

    // Is this a wire?
    auto wire = std::dynamic_pointer_cast<Wire>(_item);
    if (wire) {
        _scene->addWire(wire);
    }

    // Otherwise, fall back to normal item behavior
    else {
        _scene->addItem(_item);
    }

    // Set the item's old parent
    _item->setParentItem(_itemParent);
}

void CommandItemRemove::redo()
{
    if (!_scene or !_item) {
        return;
    }

    // Store the parent
    _itemParent = _item->parentItem();

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
