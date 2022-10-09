#pragma once

#include "commandbase.h"

#include <QPointer>
#include <QPoint>
#include <QSize>

namespace QSchematic
{
    class RectItem;

    class CommandRectItemRotate :
        public UndoCommand
    {
    public:
        CommandRectItemRotate(QPointer<RectItem> item, qreal rotation, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        void updateText();

        QPointer<RectItem> _item;
        qreal _oldAngle;
        qreal _newAngle;
    };

}
