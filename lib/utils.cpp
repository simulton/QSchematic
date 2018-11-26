#include <limits>
#include <QPoint>
#include <QPointF>
#include <QLine>
#include <QLineF>
#include <QRectF>
#include <QPainterPath>
#include <QVector2D>
#include "items/line.h"
#include "utils.h"

using namespace QSchematic;

QPoint Utils::centerPoint(const QPoint& p1, const QPoint& p2)
{
    return (p1 + p2) / 2;
}

QPointF Utils::clipPointToRect(QPointF point, const QRectF& rect)
{
    // Clip
    point.rx() = qBound(rect.x(), point.x(), rect.width());
    point.ry() = qBound(rect.y(), point.y(), rect.height());

    return point;
}

QPointF Utils::clipPointToRectOutline(QPointF point, const QRectF& rect)
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

QPointF Utils::clipPointToPath(const QPointF& point, const QPainterPath& path)
{
    Q_UNUSED(path);
#warning ToDo: Implement me

    return point;
}

QPointF Utils::pointOnLineClosestToPoint(const QPointF& p1, const QPointF& p2, const QPointF& point)
{
    // Algorithm based on: http://nic-gamedev.blogspot.ch/2011/11/using-vector-mathematics-and-bit-of_08.html
    QVector2D lineDiffVector = QVector2D(p2 - p1);
    float lineSegSqrLength = lineDiffVector.lengthSquared();

    QVector2D lineToPointVect = QVector2D(point - p1);
    float dotProduct = QVector2D::dotProduct(lineDiffVector, lineToPointVect);

    float percAlongLine = dotProduct / lineSegSqrLength;

    // Return the end points
    if (percAlongLine <= 0.0f) {
        return p1;
    } else if (percAlongLine >= 1.0f) {
        return p2;
    }

    // Return the point along the line
    return ( p1 + ( static_cast<qreal>(percAlongLine) * ( p2 - p1 )));
}

QVector<QLineF>::const_iterator Utils::lineClosestToPoint(const QVector<QLineF>& lines, const QPointF& point)
{
    // Sanity check
    if (lines.isEmpty()) {
        qFatal("Utils::lineClosestToPoint(): lines vector must not be empty");
        return nullptr;
    }

    // Figure out to which line we're closest to
    QPair<QVector<QLineF>::const_iterator, qreal> nearest(lines.constBegin(), std::numeric_limits<qreal>::max());
    for (QVector<QLineF>::const_iterator it = lines.constBegin(); it != lines.constEnd(); it++) {
        QPointF pointOnEdge = Utils::pointOnLineClosestToPoint(it->p1(), it->p2(), point);
        qreal distance = QLineF(pointOnEdge, point).length();
        if (distance < nearest.second) {
            nearest = QPair<QVector<QLineF>::const_iterator, qreal>(it, distance);
        }
    }

    // Snap to that edge
    return nearest.first;
}
