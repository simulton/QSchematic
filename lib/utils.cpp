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
    edges << QLineF(rect.topLeft(), rect.topRight());
    edges << QLineF(rect.topRight(), rect.bottomRight());
    edges << QLineF(rect.bottomRight(), rect.bottomLeft());
    edges << QLineF(rect.bottomLeft(), rect.topLeft());

    // Figure out to which line we're closest to
    QPair<QLineF, qreal> nearest(edges.first(), std::numeric_limits<qreal>::max());
    for (const QLineF& edge : edges) {
        QPointF pointOnEdge = Utils::pointOnLineClosestToPoint(edge.p1(), edge.p2(), point);
        qreal distance = QLineF(pointOnEdge, point).length();
        if (distance < nearest.second) {
            nearest = QPair<QLineF, qreal>(edge, distance);
        }
    }

    // Snap to that edge
    const QLineF& nearestEdge = nearest.first;
    point = Utils::pointOnLineClosestToPoint(nearestEdge.p1(), nearestEdge.p2(), point);

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
    if (percAlongLine <= 0.0) {
        return p1;
    } else if (percAlongLine >= 1.0) {
        return p2;
    }

    // Return the point along the line
    return ( p1 + ( percAlongLine * ( p2 - p1 )));
}
