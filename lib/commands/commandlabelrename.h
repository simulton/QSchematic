#pragma once

#include <QUndoCommand>
#include <QPointer>

namespace QSchematic
{

    class Label;

    class CommandLabelRename : public QUndoCommand
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
