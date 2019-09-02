#pragma once

#include <QPointF>

class QLineF;

namespace QSchematic {

    class Line
    {
    public:
        Line() = default;
        Line(int x1, int y1, int x2, int y2);
        Line(qreal x1, qreal y1, qreal x2, qreal y2);
        Line(const QPoint& p1, const QPoint& p2);
        Line(const QPointF& p1, const QPointF& p2);

        QPointF p1() const;
        QPointF p2() const;
        bool isNull() const;
        bool isHorizontal() const;
        bool isVertical() const;
        qreal lenght() const;
        QPointF midPoint() const;
        bool containsPoint(const QPointF& point, qreal tolerance = 0) const;
        QPointF pointOnLineClosestToPoint(const QPointF& point);

        static bool containsPoint(const QLineF& line, const QPointF& point, qreal tolerance = 0);

    private:
        QPointF _p1;
        QPointF _p2;
    };

}
