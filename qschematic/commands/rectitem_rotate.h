#pragma once

#include "base.h"

#include <QPointer>
#include <QPoint>
#include <QSize>

namespace QSchematic
{
    class RectItem;
}

namespace QSchematic::Commands
{

    class RectItemRotate :
        public Base
    {
    public:
        RectItemRotate(QPointer<RectItem> item, qreal rotation, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        QPointer<RectItem> _item;
        qreal _oldAngle;
        qreal _newAngle;
    };

}
