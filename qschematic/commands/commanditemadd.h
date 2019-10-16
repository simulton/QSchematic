#pragma once

#include "commandbase.h"
#include <QPointer>
#include <memory>

namespace QSchematic
{
    class Scene;
    class Item;

    class CommandItemAdd : public UndoCommand
    {
    public:
        CommandItemAdd(const QPointer<Scene>& scene, const std::shared_ptr<Item>& item, QUndoCommand* parent = nullptr);

        virtual int id() const  override;
        virtual bool mergeWith(const QUndoCommand* command)  override;
        virtual void undo()  override;
        virtual void redo()  override;

    private:
        QPointer<Scene> _scene;
        std::shared_ptr<Item> _item;
    };

}
