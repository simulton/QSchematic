#pragma once

#include <memory>
#include "commandbase.h"
#include <QPointer>

class QGraphicsItem;

namespace QSchematic
{
    class Scene;
    class Item;

    class QSCHEMATIC_EXPORT CommandItemRemove :
        public UndoCommand
    {
    public:
        CommandItemRemove(const QPointer<Scene>& scene, const std::shared_ptr<Item>& item, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        QPointer<Scene> _scene;
        std::shared_ptr<Item> _item;
        QGraphicsItem* _itemParent;
    };

}
