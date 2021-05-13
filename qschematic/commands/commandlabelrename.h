#pragma once

#include "commandbase.h"
#include <QPointer>

namespace QSchematic
{

    class Label;

    class QSCHEMATIC_EXPORT CommandLabelRename :
        public UndoCommand
    {
    public:
        CommandLabelRename(const QPointer<Label>& label, const QString& newText, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        QPointer<Label> _label;
        QString _oldText;
        QString _newText;
    };

}
