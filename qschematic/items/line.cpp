#include <QtGlobal>
#include <QLineF>
#include <QVector2D>
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

bool Line::containsPoint(const QPointF& point, qreal tolerance) const
{
    return containsPoint(QLineF(_p1, _p2), point, tolerance);
}

QPointF Line::pointOnLineClosestToPoint(const QPointF& point)
{
    return Utils::pointOnLineClosestToPoint(_p1, _p2, point);
}

bool Line::containsPoint(const QLineF& line, const QPointF& point, qreal tolerance)
{
    const qreal MIN_LENGTH = 0.01;
    tolerance = qMax(static_cast<qreal>(tolerance), MIN_LENGTH);

    // Find perpendicular line
    QLineF normal = line.normalVector();
    // Move line to point
    QPointF offset = point - normal.p1();
    normal.translate(offset);
    // Set length to double the tolerance
    normal.setLength(2*tolerance);
    // Move line so that the center lays on the point
    QVector2D unit(normal.unitVector().dx(), normal.unitVector().dy());
    offset = (unit * -tolerance).toPointF();
    normal.translate(offset);

    // Check if the lines are intersecting
    if (line.intersect(normal, nullptr) == QLineF::BoundedIntersection) {
        return true;
    }

    return false;
}
