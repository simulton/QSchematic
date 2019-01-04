#pragma once

#include <QPointF>

namespace QSchematic {

    class WirePoint : private QPointF
    {
    public:
        // Expose some of the QPoint interfaces
        using QPointF::setX;
        using QPointF::setY;
        using QPointF::x;
        using QPointF::y;
        using QPointF::rx;
        using QPointF::ry;
        using QPointF::toPoint;

        WirePoint();
        WirePoint(const WirePoint& other);
        WirePoint(const QPoint& point);
        WirePoint(const QPointF& point);
        WirePoint(int x, int y);
        WirePoint(qreal x, qreal y);
        virtual ~WirePoint() = default;

        void setIsJunction(bool isJunction);
        bool isJunction() const;

    private:
        bool _isJunction;
    };
}

bool operator==(const QSchematic::WirePoint& a, const QSchematic::WirePoint& b);
bool operator==(const QSchematic::WirePoint& a, const QPoint& b);
bool operator==(const QSchematic::WirePoint& a, const QPointF& b);
const QPoint operator+(const QSchematic::WirePoint& a, const QPoint& b);
const QPointF operator+(const QSchematic::WirePoint& a, const QPointF& b);
