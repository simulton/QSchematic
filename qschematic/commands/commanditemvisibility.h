#pragma once

#include "commandbase.h"

#include <memory>

namespace QSchematic
{
    class Item;
}

namespace QSchematic::Commands
{

    class Item;

    class ItemVisibility :
        public Base
    {
    public:
        ItemVisibility(const std::shared_ptr<QSchematic::Item>& item, bool newVisibility, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        std::shared_ptr<QSchematic::Item> _item;
        bool _oldVisibility;
        bool _newVisibility;
    };

}
