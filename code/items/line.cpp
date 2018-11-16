#include <QtGlobal>
#include <QLineF>
#include <QVector2D>
#include "line.h"

using namespace QSchematic;

Line::Line(int x1, int y1, int x2, int y2) :
    _p1(QPoint(x1, y1)),
    _p2(QPoint(x2, y2))
{
}

Line::Line(const QPoint& p1, const QPoint& p2) :
    _p1(p1),
    _p2(p2)
{
}

QPoint Line::p1() const
{
    return _p1;
}

QPoint Line::p2() const
{
    return _p2;
}

bool Line::isNull() const
{
    return (_p1.x() == _p2.x()) && (_p1.y() == _p2.y());
}

bool Line::isHorizontal() const
{
    return (_p1.y() == _p2.y());
}

bool Line::isVertical() const
{
    return (_p1.x() == _p2.x());
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

QPointF Line::closestPointTo(const QPointF& point)
{
    return closestPointTo(QLineF(_p1, _p2), point);
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

QPointF Line::closestPointTo(const QLineF& line, const QPointF& point)
{
    // Algorithm based on: http://nic-gamedev.blogspot.ch/2011/11/using-vector-mathematics-and-bit-of_08.html
    QVector2D lineDiffVector = QVector2D(line.p2() - line.p1());
    double lineSegSqrLength = lineDiffVector.lengthSquared();

    QVector2D lineToPointVect = QVector2D(point - line.p1());
    double dotProduct = QVector2D::dotProduct(lineDiffVector, lineToPointVect);

    double percAlongLine = dotProduct / lineSegSqrLength;

    // Return the end points
    if (percAlongLine <= 0.0) {
        return line.p1();
    } else if (percAlongLine >= 1.0) {
        return line.p2();
    }

    // Return the point along the line
    return ( line.p1() + ( percAlongLine * ( line.p2() - line.p1() )));
}
