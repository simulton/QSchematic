#include <QPointF>
#include <QRectF>
#include <QPainterPath>
#include "settings.h"

using namespace QSchematic;

Settings::Settings() :
    debug(false),
    gridSize(20),
    gridPointSize(3),
    showGrid(true),
    highlightRectPadding(10),
    resizeHandleSize(9),
    routeStraightAngles(true),
    antialiasing(true),
    popupDelay(400)
{
}

QPoint Settings::toGridPoint(const QPointF& point) const
{
    int gridX = qRound(point.x() / gridSize);
    int gridY = qRound(point.y() / gridSize);

    return QPoint(gridX, gridY);
}

QPoint Settings::toScenePoint(const QPoint& gridCoordinate) const
{
    return gridCoordinate * gridSize;
}

QPoint Settings::snapToGridPoint(const QPointF& scenePoint) const
{
    int xV = qRound(scenePoint.x() / gridSize) * gridSize;
    int yV = qRound(scenePoint.y() / gridSize) * gridSize;

    return QPoint(xV, yV);
}

QPoint Settings::centerPoint(const QPoint& p1, const QPoint& p2)
{
    return (p1 + p2) / 2;
}

QPointF Settings::clipPointToRect(QPointF point, const QRectF& rect)
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

QPointF Settings::clipPointToRectOutline(QPointF point, const QRectF& rect)
{
    return point;
}

QPointF Settings::clipPointToPath(const QPointF& point, const QPainterPath& path)
{
    Q_UNUSED(path);
#warning ToDo: Implement me

    return point;
}
