#include "../items/node.h"
#include "../scene.h"
#include "commands.h"
#include "commanditemremove.h"

using namespace QSchematic;

CommandItemRemove::CommandItemRemove(const QPointer<Scene>& scene, Item* item, QUndoCommand* parent) :
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

    _scene->addItem(_item);
    _item->setParentItem(_itemParent);
}

void CommandItemRemove::redo()
{
    if (!_scene or !_item) {
        return;
    }

    _itemParent = _item->parentItem();
    _scene->removeItem(_item);
}
