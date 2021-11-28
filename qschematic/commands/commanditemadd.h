#pragma once

#include "commandbase.h"

#include <QPointer>
#include <memory>

namespace QSchematic
{
    class Scene;
    class Item;

    class QSCHEMATIC_EXPORT CommandItemAdd :
        public UndoCommand
    {
    public:
        CommandItemAdd(const QPointer<Scene>& scene, const std::shared_ptr<Item>& item, QUndoCommand* parent = nullptr);

        int id() const  override;
        bool mergeWith(const QUndoCommand* command)  override;
        void undo()  override;
        void redo()  override;

    private:
        QPointer<Scene> _scene;
        std::shared_ptr<Item> _item;
    };

}
