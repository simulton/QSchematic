#pragma once

#include <QUndoCommand>
#include <QPointer>
#include <QPoint>
#include <QSize>

namespace QSchematic
{
    class Node;

    class CommandNodeResize : public QUndoCommand
    {
    public:
        CommandNodeResize(QPointer<Node> node, const QPointF& newPos, const QSizeF& newSize, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        void updateText();

        QPointer<Node> _node;
        QPointF _oldPos;
        QPointF _newPos;
        QSizeF _oldSize;
        QSizeF _newSize;
    };

}
