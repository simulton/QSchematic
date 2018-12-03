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
        CommandNodeResize(QPointer<Node> node, const QPoint& newGridPos, const QSize& newSize, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        void updateText();

        QPointer<Node> _node;
        QPoint _oldGridPos;
        QPoint _newGridPos;
        QSize _oldSize;
        QSize _newSize;
    };

}
