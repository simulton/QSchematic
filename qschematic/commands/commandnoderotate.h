#pragma once

#include "commandbase.h"
#include <QPointer>
#include <QPoint>
#include <QSize>

namespace QSchematic
{
    class Node;

    class QSCHEMATIC_EXPORT CommandNodeRotate :
        public UndoCommand
    {
    public:
        CommandNodeRotate(QPointer<Node> node, qreal rotation, QUndoCommand* parent = nullptr);

        virtual int id() const override;
        virtual bool mergeWith(const QUndoCommand* command) override;
        virtual void undo() override;
        virtual void redo() override;

    private:
        void updateText();

        QPointer<Node> _node;
        qreal _oldAngle;
        qreal _newAngle;
    };

}
