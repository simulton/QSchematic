#pragma once

#include <QUndoCommand>
#include <memory>

namespace QSchematic
{

    class Label;

    class CommandLabelRename : public QUndoCommand
    {
    public:
        CommandLabelRename(const std::shared_ptr<Label>& label, const QString& newText, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        std::shared_ptr<Label> _label;
        QString _oldText;
        QString _newText;
    };

}
