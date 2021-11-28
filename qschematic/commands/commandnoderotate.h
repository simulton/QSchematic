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

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        void updateText();

        QPointer<Node> _node;
        qreal _oldAngle;
        qreal _newAngle;
    };

}
