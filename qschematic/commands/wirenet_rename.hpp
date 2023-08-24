#pragma once

#include "base.hpp"

#include <memory>

namespace QSchematic::Items
{
    class Label;
    class WireNet;
}

namespace QSchematic::Commands
{

    class WirenetRename :
        public Base
    {
    public:
        WirenetRename(const std::shared_ptr<Items::WireNet>& net, const QString& newText, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        std::shared_ptr<Items::WireNet> _net;
        QString _oldText;
        QString _newText;
    };

}
