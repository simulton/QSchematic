#pragma once

#include "base.hpp"

#include <QPointer>
#include <QPoint>
#include <QSize>

namespace QSchematic::Items
{
    class RectItem;
}

namespace QSchematic::Commands
{

    class RectItemResize :
        public Base
    {
    public:
        RectItemResize(QPointer<Items::RectItem> item, const QPointF& newPos, const QSizeF& newSize, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        QPointer<Items::RectItem> _item;
        QPointF _oldPos;
        QPointF _newPos;
        QSizeF _oldSize;
        QSizeF _newSize;
    };

}
