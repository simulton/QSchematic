#pragma once

#include <QUndoCommand>
#include <QPointer>

namespace QSchematic
{
    class Item;

    class CommandItemSetVisible : public QUndoCommand
    {
    public:
        CommandItemSetVisible(const QPointer<Item>& item, bool newVisibility, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        QPointer<Item> _item;
        bool _oldVisibility;
        bool _newVisibility;
    };

}
