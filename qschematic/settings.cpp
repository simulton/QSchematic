#include "settings.h"

#include <QPointF>
#include <QRectF>
#include <QVector2D>

using namespace QSchematic;

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

QPoint Settings::snapToGrid(const QPointF& scenePoint) const
{
    int xV = qRound(scenePoint.x() / gridSize) * gridSize;
    int yV = qRound(scenePoint.y() / gridSize) * gridSize;

    return QPoint(xV, yV);
}

QVector2D Settings::snapToGrid(const QVector2D& sceneVector) const
{
    int xV = qRound(sceneVector.x() / gridSize) * gridSize;
    int yV = qRound(sceneVector.y() / gridSize) * gridSize;

    return QVector2D(xV, yV);
}

QSize Settings::snapToGrid(const QSizeF& sceneSize) const
{
    int w = qRound(sceneSize.width() / gridSize) * gridSize;
    int h = qRound(sceneSize.height() / gridSize) * gridSize;

    return QSize(w, h);
}
