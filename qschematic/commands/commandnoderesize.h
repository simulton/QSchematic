#pragma once

#include "commandbase.h"
#include <QPointer>
#include <QPoint>
#include <QSize>

namespace QSchematic
{
    class Node;

    class QSCHEMATIC_EXPORT CommandNodeResize :
        public UndoCommand
    {
    public:
        CommandNodeResize(QPointer<Node> node, const QPointF& newPos, const QSizeF& newSize, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        void updateText();

        QPointer<Node> _node;
        QPointF _oldPos;
        QPointF _newPos;
        QSizeF _oldSize;
        QSizeF _newSize;
    };

}
