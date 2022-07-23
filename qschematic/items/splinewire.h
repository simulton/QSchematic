#pragma once

#include "wire.h"

namespace QSchematic
{
    class SplineWire :
        public QSchematic::Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(SplineWire)
    public:
        SplineWire(int type = Item::SplineWireType, QGraphicsItem* parent = nullptr);
        ~SplineWire() override = default;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
        QPainterPath path() const;
        QPainterPath shape() const override;
        QRectF boundingRect() const override;
    };
}
