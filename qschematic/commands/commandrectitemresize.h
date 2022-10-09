#pragma once

#include "commandbase.h"

#include <QPointer>
#include <QPoint>
#include <QSize>

namespace QSchematic
{
    class RectItem;

    class CommandRectItemResize :
        public UndoCommand
    {
    public:
        CommandRectItemResize(QPointer<RectItem> item, const QPointF& newPos, const QSizeF& newSize, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        void updateText();

        QPointer<RectItem> _item;
        QPointF _oldPos;
        QPointF _newPos;
        QSizeF _oldSize;
        QSizeF _newSize;
    };

}
