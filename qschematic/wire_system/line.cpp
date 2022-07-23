#include "line.h"
#include "../utils.h"

#include <QtGlobal>
#include <QLineF>
#include <QVector2D>

using namespace wire_system;

line::line(int x1, int y1, int x2, int y2) :
    m_p1(QPointF(x1, y1)),
    m_p2(QPointF(x2, y2))
{
}

line::line(qreal x1, qreal y1, qreal x2, qreal y2) :
    m_p1(QPointF(x1, y1)),
    m_p2(QPointF(x2, y2))
{
}

line::line(const QPoint& p1, const QPoint& p2) :
    m_p1(p1),
    m_p2(p2)
{
}

line::line(const QPointF& p1, const QPointF& p2) :
    m_p1(p1),
    m_p2(p2)
{
}

QPointF line::p1() const
{
    return m_p1;
}

QPointF line::p2() const
{
    return m_p2;
}

bool line::is_null() const
{
    return qFuzzyCompare(m_p1.x(), m_p2.x()) && qFuzzyCompare(m_p1.y(), m_p2.y());
}

bool line::is_horizontal() const
{
    return qFuzzyCompare(m_p1.y(), m_p2.y());
}

bool line::is_vertical() const
{
    return qFuzzyCompare(m_p1.x(), m_p2.x());
}

qreal line::lenght() const
{
    return ::QLineF(m_p1, m_p2).length();
}

QPointF line::mid_point() const
{
    return (m_p1 + m_p2) / 2;
}

bool line::contains_point(const QPointF& point, qreal tolerance) const
{
    return contains_point(QLineF(m_p1, m_p2), point, tolerance);
}

QPointF line::point_on_line_closest_to(const QPointF& point)
{
    return QSchematic::Utils::pointOnLineClosestToPoint(m_p1, m_p2, point);
}

QLineF line::toLineF() const
{
    return QLineF(m_p1, m_p2);
}

bool line::contains_point(const QLineF& line, const QPointF& point, qreal tolerance)
{
    const qreal MIN_LENGTH = 0.01;
    tolerance = qMax(tolerance, MIN_LENGTH);

    if (line.isNull()) {
        QPointF linePoint = line.p1();
        if (QVector2D(linePoint).distanceToPoint(QVector2D(point)) <= tolerance) {
            return true;
        }
    } else {
        // Find perpendicular line
        QLineF normal = line.normalVector();
        // Move line to point
        QPointF offset = point - normal.p1();
        normal.translate(offset);
        // Set length to double the tolerance
        normal.setLength(2 * tolerance);
        // Move line so that the center lays on the point
        QVector2D unit(normal.unitVector().dx(), normal.unitVector().dy());
        offset = (unit * -tolerance).toPointF();
        normal.translate(offset);
        // Make the line longer by 2 * tolerance
        QLineF lineAdjusted = line;
        lineAdjusted.setLength(line.length() + 2 * tolerance);

        // Check if the lines are intersecting
#       if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        if (lineAdjusted.intersects(normal, nullptr) == QLineF::BoundedIntersection) {
#       else
        if (lineAdjusted.intersect(normal, nullptr) == QLineF::BoundedIntersection) {
#       endif
            return true;
        }
    }

    return false;
}
