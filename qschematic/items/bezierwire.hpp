#pragma once

#include "wire.hpp"

namespace QSchematic::Items
{

    class BezierWire :
        public QSchematic::Items::Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(BezierWire)

    public:
        explicit
        BezierWire(int type = Item::BezierWireType, QGraphicsItem* parent = nullptr);

        ~BezierWire() override = default;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
        QPainterPath path() const;
        QPainterPath shape() const override;
        QRectF boundingRect() const override;
    };

}
