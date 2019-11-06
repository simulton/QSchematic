#pragma once

#include "commandbase.h"
#include <memory>

namespace QSchematic
{

    class Item;

    class CommandItemVisibility : public UndoCommand
    {
    public:
        CommandItemVisibility(const std::shared_ptr<Item>& item, bool newVisibility, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        std::shared_ptr<Item> _item;
        bool _oldVisibility;
        bool _newVisibility;
    };

}
