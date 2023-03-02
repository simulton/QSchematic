#include "wire_system/line.h"
#include "utils.h"

#include <QPoint>
#include <QLine>
#include <QRectF>
#include <QPainterPath>
#include <QVector2D>

#include <limits>

using namespace QSchematic;

QPoint
Utils::centerPoint(const QPoint& p1, const QPoint& p2)
{
    return (p1 + p2) / 2;
}

QPointF
Utils::centerPoint(const QPointF& p1, const QPointF& p2)
{
    return (p1 + p2) / 2;
}

QPointF
Utils::clipPointToRect(QPointF point, const QRectF& rect)
{
    // Clip
    point.rx() = qBound(rect.x(), point.x(), rect.width());
    point.ry() = qBound(rect.y(), point.y(), rect.height());

    return point;
}

QPointF
Utils::clipPointToRectOutline(QPointF point, const QRectF& rect)
{
    // Create list of edges
    QVector<QLineF> edges(4);
    edges[0] = QLineF(rect.topLeft(), rect.topRight());
    edges[1] = QLineF(rect.topRight(), rect.bottomRight());
    edges[2] = QLineF(rect.bottomRight(), rect.bottomLeft());
    edges[3] = QLineF(rect.bottomLeft(), rect.topLeft());

    // Figure out to which edge we're closest to
    const auto& nearestEdge = Utils::lineClosestToPoint(edges, point);
    Q_ASSERT(nearestEdge);

    // Snap to that edge
    point = Utils::pointOnLineClosestToPoint(nearestEdge->p1(), nearestEdge->p2(), point);

    return point;
}

QPointF
Utils::pointOnLineClosestToPoint(const QPointF& p1, const QPointF& p2, const QPointF& point)
{
    // Algorithm based on: http://nic-gamedev.blogspot.ch/2011/11/using-vector-mathematics-and-bit-of_08.html
    QVector2D lineDiffVector = QVector2D(p2 - p1);
    float lineSegSqrLength = lineDiffVector.lengthSquared();

    QVector2D lineToPointVect = QVector2D(point - p1);
    float dotProduct = QVector2D::dotProduct(lineDiffVector, lineToPointVect);

    float percAlongLine = dotProduct / lineSegSqrLength;

    // Return the end points
    if (percAlongLine <= 0.0f)
        return p1;
    else if (percAlongLine >= 1.0f)
        return p2;

    // Return the point along the line
    return ( p1 + ( static_cast<qreal>(percAlongLine) * ( p2 - p1 )));
}

QVector<QLineF>::const_iterator
Utils::lineClosestToPoint(const QVector<QLineF>& lines, const QPointF& point)
{
    // Sanity check
    if (lines.isEmpty())
        return { };

    // Figure out to which line we're closest to
    QPair<QVector<QLineF>::const_iterator, qreal> nearest(lines.constBegin(), std::numeric_limits<qreal>::max());
    for (QVector<QLineF>::const_iterator it = lines.constBegin(); it != lines.constEnd(); it++) {
        QPointF pointOnEdge = Utils::pointOnLineClosestToPoint(it->p1(), it->p2(), point);
        qreal distance = QLineF(pointOnEdge, point).length();
        if (distance < nearest.second)
            nearest = QPair<QVector<QLineF>::const_iterator, qreal>(it, distance);
    }

    // Snap to that edge
    return nearest.first;
}

QVector<QPointF>
Utils::rectanglePoints(const QRectF& rect, RectanglePointTypes pointTypes)
{
    QVector<QPointF> points;

    // Add corners (if supposed to)
    if (pointTypes & RectangleCornerPoints) {
        points.reserve(4);
        points.append(rect.topLeft());
        points.append(rect.topRight());
        points.append(rect.bottomLeft());
        points.append(rect.bottomRight());
    }

    // Add edges center points (if supposed to)
    if (pointTypes & RectangleEdgeCenterPoints) {
        points.reserve(4);
        points.append(Utils::centerPoint(rect.topLeft(), rect.topRight()));
        points.append(Utils::centerPoint(rect.topRight(), rect.bottomRight()));
        points.append(Utils::centerPoint(rect.bottomRight(), rect.bottomLeft()));
        points.append(Utils::centerPoint(rect.bottomLeft(), rect.topLeft()));
    }

    return points;
}

bool
Utils::lineIsHorizontal(const QPointF& p1, const QPointF& p2)
{
    return qFuzzyCompare(p1.y(), p2.y());
}

bool
Utils::lineIsVertical(const QPointF& p1, const QPointF& p2)
{
    return qFuzzyCompare(p1.x(), p2.x());
}

bool
Utils::pointIsOnLine(const QLineF& line, const QPointF& point)
{
    /*
     * Convert both two neighbouring points to a translation vector (i.e. substract the second from the first)
     * and calculate their dot product. If the dot product is 0 they're orthogonal.
     * if they're on exactly the same line p.q = |p|*|q|
     */

    QVector2D v1(line.p2() - line.p1());
    QVector2D v2(point - line.p2());

    float dotProduct = QVector2D::dotProduct(v1, v2);
    float absProduct =  v1.length() * v2.length();

    return qFuzzyCompare(dotProduct, absProduct);
}

std::vector<QLineF>
Utils::linesFromPoints(const QVector<QPointF>& points, const bool closeLoop)
{
    if (points.size() < 2)
        return { };

    const std::size_t lines_cnt = (closeLoop ? points.size() : points.size()-1);
    std::vector<QLineF> lines{lines_cnt};
    for (std::size_t i = 0; i < points.size()-1; i++) {
        lines[i] = QLineF{points.at(i), points.at(i+1)};
    }
    if (closeLoop)
        lines[lines_cnt-1] = QLineF{points.at(points.size()-1), points.at(0)};

    return lines;
}

std::optional<QPointF>
Utils::intersectionPoint(const QRectF& rect, const QLineF& line)
{
    // Get the corner points of the rectangle
    // The order here matters - we need to generate a continuous (sorted) loop!
    QVector<QPointF> rp{4};
    rp[0] = rect.topLeft();
    rp[1] = rect.topRight();
    rp[2] = rect.bottomRight();
    rp[3] = rect.bottomLeft();

    for (const QLineF& rect_line : linesFromPoints(rp, true)) {
        QPointF point;
        const auto type = line.intersects(rect_line, &point);
        if (type == QLineF::IntersectionType::BoundedIntersection)
            return point;
    }

    return { };
}
