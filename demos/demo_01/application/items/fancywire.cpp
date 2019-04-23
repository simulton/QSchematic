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

Gds::Container FancyWire::toContainer() const
{
    // Root
    Gds::Container root;
    addItemTypeIdToContainer(root);
    root.addEntry("wire", QSchematic::Wire::toContainer());

    return root;
}

void FancyWire::fromContainer(const Gds::Container& container)
{
    QSchematic::Wire::fromContainer( container.getEntry<Gds::Container>( "wire" ) );
}

std::unique_ptr<QSchematic::Item> FancyWire::deepCopy() const
{
    auto clone = std::make_unique<FancyWire>(parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void FancyWire::copyAttributes(FancyWire& dest) const
{
    QSchematic::WireRoundedCorners::copyAttributes(dest);
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
    const auto& points = pointsAbsolute();
    auto it = points.constBegin();
    while (it != points.constEnd()) {
        const auto& point = it->toPoint();

        if (connectionPoints.contains(point)) {
            painter->drawEllipse(point, SIZE, SIZE);
        }

        it++;
    }
}
