#pragma once

#include "wire.h"

namespace QSchematic
{
    class SplineWire : public QSchematic::Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY(SplineWire)
    public:
        SplineWire(int type = Item::SplineWireType, QGraphicsItem* parent = nullptr);
        virtual ~SplineWire() override = default;

        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
        virtual QPainterPath path() const;
        virtual QPainterPath shape() const override;
        virtual QRectF boundingRect() const override;
    };
}
