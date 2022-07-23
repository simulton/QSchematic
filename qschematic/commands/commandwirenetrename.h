#pragma once

#include "commandbase.h"
#include "../items/wirenet.h"

#include <memory>

namespace QSchematic
{

    class Label;

    class CommandWirenetRename :
        public UndoCommand
    {
    public:
        CommandWirenetRename(const std::shared_ptr<WireNet>& net, const QString& newText, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        std::shared_ptr<WireNet> _net;
        QString _oldText;
        QString _newText;
    };

}
