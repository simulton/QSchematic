#include "commands.hpp"
#include "item_add.hpp"
#include "../items/item.hpp"
#include "../items/wire.hpp"
#include "../scene.hpp"

using namespace QSchematic;
using namespace QSchematic::Commands;

ItemAdd::ItemAdd(const QPointer<Scene>& scene, const std::shared_ptr<Items::Item>& item, QUndoCommand* parent) :
    Base(parent),
    _scene(scene),
    _item(item)
{
    connectDependencyDestroySignal(_scene.data());
    setText(tr("Add item"));
}

int
ItemAdd::id() const
{
    return ItemAddCommandType;
}

bool
ItemAdd::mergeWith(const QUndoCommand* command)
{
    Q_UNUSED(command)

    return false;
}

void
ItemAdd::undo()
{
    if (!_scene || !_item)
        return;

    // Is this a wire?
    auto wire = std::dynamic_pointer_cast<Items::Wire>(_item);
    if (wire)
        _scene->removeWire(wire);

    // Otherwise, fall back to normal item behavior
    else
        _scene->removeItem(_item);
}

void
ItemAdd::redo()
{
    if (!_scene || !_item)
        return;

    // Is this a wire?
    auto wire = std::dynamic_pointer_cast<Items::Wire>(_item);
    if (wire) {
        if (wire->net()) {
            if (!_scene->wire_manager()->nets().contains(wire->net()))
                _scene->wire_manager()->add_net(wire->net());

            wire->net()->addWire(wire);
            _scene->addItem(wire);
        }
        else
            _scene->addWire(wire);

        for (int i = 0; i < wire->wirePointsRelative().count(); i++)
            _scene->wire_manager()->point_moved_by_user(*wire.get(), i);
    }

    // Otherwise, fall back to normal item behavior
    else
        _scene->addItem(_item);
}
