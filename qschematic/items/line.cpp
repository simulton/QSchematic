#include <QtGlobal>
#include <QLineF>
#include "line.h"
#include "../utils.h"

using namespace QSchematic;

Line::Line(int x1, int y1, int x2, int y2) :
    _p1(QPointF(x1, y1)),
    _p2(QPointF(x2, y2))
{
}

Line::Line(qreal x1, qreal y1, qreal x2, qreal y2) :
    _p1(QPointF(x1, y1)),
    _p2(QPointF(x2, y2))
{
}

Line::Line(const QPoint& p1, const QPoint& p2) :
    _p1(p1),
    _p2(p2)
{
}

Line::Line(const QPointF& p1, const QPointF& p2) :
    _p1(p1),
    _p2(p2)
{
}

QPointF Line::p1() const
{
    return _p1;
}

QPointF Line::p2() const
{
    return _p2;
}

bool Line::isNull() const
{
    return qFuzzyCompare(_p1.x(), _p2.x()) && qFuzzyCompare(_p1.y(), _p2.y());
}

bool Line::isHorizontal() const
{
    return qFuzzyCompare(_p1.y(), _p2.y());
}

bool Line::isVertical() const
{
    return qFuzzyCompare(_p1.x(), _p2.x());
}

qreal Line::lenght() const
{
    return ::QLineF(_p1, _p2).length();
}

QPointF Line::midPoint() const
{
    return (_p1 + _p2) / 2;
}

bool Line::containsPoint(const QPointF& point, unsigned tolerance) const
{
    return containsPoint(QLineF(_p1, _p2), point, tolerance);
}

QPointF Line::pointOnLineClosestToPoint(const QPointF& point)
{
    return Utils::pointOnLineClosestToPoint(_p1, _p2, point);
}

bool Line::containsPoint(const QLineF& line, const QPointF& point, unsigned tolerance)
{
    const qreal MIN_LENGTH = 0.01;
    QLineF imaginaryLine(point.x()-tolerance-MIN_LENGTH, point.y()-tolerance-MIN_LENGTH, point.x()+tolerance+MIN_LENGTH, point.y()+tolerance+MIN_LENGTH);

    if (line.intersect(imaginaryLine, nullptr) == QLineF::BoundedIntersection) {
        return true;
    }

    return false;
}
