#pragma once

#include "commandbase.h"

#include <QPointer>

namespace QSchematic
{
    class Label;
}

namespace QSchematic::Commands
{

    class LabelRename :
        public Base
    {
    public:
        LabelRename(const QPointer<Label>& label, const QString& newText, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        QPointer<Label> _label;
        QString _oldText;
        QString _newText;
    };

}
