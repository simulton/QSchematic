#pragma once

#include <QPoint>

class QLineF;

namespace QSchematic {

    class Line
    {
    public:
        Line() = default;
        Line(int x1, int y1, int x2, int y2);
        Line(const QPoint& p1, const QPoint& p2);

        QPoint p1() const;
        QPoint p2() const;
        bool isNull() const;
        bool isHorizontal() const;
        bool isVertical() const;
        qreal lenght() const;
        QPointF midPoint() const;
        bool containsPoint(const QPointF& point, unsigned tolerance = 0) const;
        QPointF pointOnLineClosestToPoint(const QPointF& point);

        static bool containsPoint(const QLineF& line, const QPointF& point, unsigned tolerance = 0);

    private:
        QPoint _p1;
        QPoint _p2;
    };

}
