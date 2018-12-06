#pragma once

#include <QUndoCommand>
#include <QVector>
#include <QPointer>
#include <QVector2D>

class QVector2D;

namespace QSchematic
{
    class Item;

    class CommandItemMove : public QUndoCommand
    {
    public:
        CommandItemMove(const QVector<QPointer<Item>>& item, const QVector2D& moveBy, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        QVector<QPointer<Item>> _items;
        QVector2D _moveBy;
    };

}
