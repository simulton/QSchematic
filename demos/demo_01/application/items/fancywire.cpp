#include <QJsonObject>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include "../../../lib/items/wirepoint.h"
#include "../../../lib/scene.h"
#include "../../../lib/settings.h"
#include "itemtypes.h"
#include "fancywire.h"

#define SIZE (_settings.gridSize/3)

FancyWire::FancyWire(QGraphicsItem* parent) :
    QSchematic::WireRoundedCorners(::ItemType::FancyWireType, parent)
{
    setZValue(1);
}

QJsonObject FancyWire::toJson() const
{
    QJsonObject object;

    object.insert("wire", QSchematic::Wire::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool FancyWire::fromJson(const QJsonObject& object)
{
    QSchematic::Wire::fromJson(object["wire"].toObject());

    return true;
}

void FancyWire::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Base class
    QSchematic::WireRoundedCorners::paint(painter, option, widget);

    // Nothing to do if we can't retrieve a list of all available connection points
    if (!scene()) {
        return;
    }

    QPen pen(Qt::NoPen);

    QBrush brush;
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);

    // Retrieve a list of all available connection points in the scene
    const auto& connectionPoints = scene()->connectionPoints();

    // Make points fancy if they are on top of one of our connectors
    painter->setPen(pen);
    painter->setBrush(brush);
    const auto& wirePoints = sceneWirePointsAbsolute();
    auto it = wirePoints.constBegin();
    while (it != wirePoints.constEnd()) {
        const auto& wirePoint = it->toPoint();

        if (connectionPoints.contains(_settings.toGridPoint(wirePoint))) {
            painter->drawEllipse(wirePoint - gridPos(), SIZE, SIZE);
        }

        it++;
    }
}
