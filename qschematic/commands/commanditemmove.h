#pragma once

#include "commandbase.h"

#include <QVector>
#include <QVector2D>

#include <memory>

class QVector2D;

namespace QSchematic
{
    class Item;

    class CommandItemMove :
        public UndoCommand
    {
    public:
        CommandItemMove(const QVector<std::shared_ptr<Item>>& item, const QVector<QVector2D>& moveBy, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        QVector<std::shared_ptr<Item>> _items;
        QVector<QVector2D> _moveBy;
        void simplifyWires() const;
    };

}
