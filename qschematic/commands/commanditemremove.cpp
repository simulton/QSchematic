#include "commands.h"
#include "commanditemremove.h"
#include "../items/item.h"
#include "../items/wire.h"
#include "../scene.h"

using namespace QSchematic;

CommandItemRemove::CommandItemRemove(const QPointer<Scene>& scene, const std::shared_ptr<Item>& item, QUndoCommand* parent) :
    UndoCommand(parent),
    _scene(scene),
    _item(item)
{
    Q_ASSERT(scene);
    connectDependencyDestroySignal(_scene.data());
    // Everything needs a text, right?
    setText(tr("Remove item"));
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
    if (!_scene || !_item) {
        return;
    }

    _scene->addItem(_item);

    // Is this a wire?
    if ( auto wire = std::dynamic_pointer_cast<Wire>(_item) ) {
        auto oldNet = wire->net();
        if (!_scene->wire_manager()->nets().contains(oldNet)) {
            _scene->wire_manager()->add_net(wire->net());
        }

        wire->net()->addWire(wire);
        for (int i = 0; i < wire->wirePointsRelative().count(); i++) {
            _scene->wire_manager()->point_moved_by_user(*wire.get(), i);
        }
    }

    // Set the item's old parent
    _item->setParentItem(_itemParent);
}

void CommandItemRemove::redo()
{
    if (!_scene || !_item) {
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
