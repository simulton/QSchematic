#pragma once

#include <QUndoCommand>
#include <QPointer>

class QGraphicsItem;

namespace QSchematic
{
    class Scene;
    class Item;

    class CommandItemRemove : public QUndoCommand
    {
    public:
        CommandItemRemove(const QPointer<Scene>& scene, Item* item, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        QPointer<Scene> _scene;
        Item* _item;
        QGraphicsItem* _itemParent;
    };

}
