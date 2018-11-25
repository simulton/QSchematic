#include <QPoint>
#include <QPointF>
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
    // Clip X
    if (point.x() < rect.x()) {
        point.rx() = rect.x();
    } else if (point.x() > rect.width()) {
        point.rx() = rect.width();
    }

    // Clip Y
    if (point.y() < rect.y()) {
        point.ry() = rect.y();
    } else if (point.y() > rect.height()) {
        point.ry() = rect.height();
    }

    return point;
}

QPointF Utils::clipPointToRectOutline(QPointF point, const QRectF& rect)
{
    // Clip to rect
    point = Utils::clipPointToRect(point, rect);



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
