#pragma once

#include "commandbase.h"
#include <memory>

namespace QSchematic
{

    class Item;

    class QSCHEMATIC_EXPORT CommandItemVisibility :
        public UndoCommand
    {
    public:
        CommandItemVisibility(const std::shared_ptr<Item>& item, bool newVisibility, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        std::shared_ptr<Item> _item;
        bool _oldVisibility;
        bool _newVisibility;
    };

}
