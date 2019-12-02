#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QVector2D>
#include <QInputDialog>
#include "../qschematic/commands/commandwirenetrename.h"
#include "../qschematic/items/wirepoint.h"
#include "../qschematic/items/connector.h"
#include "../qschematic/items/wirenet.h"
#include "../qschematic/scene.h"
#include "../qschematic/settings.h"
#include "itemtypes.h"
#include "fancywire.h"

#define SIZE (_settings.gridSize/3)

FancyWire::FancyWire(QGraphicsItem* parent) :
    QSchematic::WireRoundedCorners(::ItemType::FancyWireType, parent)
{
    auto action = new QAction("Rename ...", this);
    connect(action, &QAction::triggered, this, [=] {
        QString name = QInputDialog::getText(nullptr, "Set WireNet name", "Enter the new name", QLineEdit::Normal, net()->name());
        scene()->undoStack()->push(new QSchematic::CommandWirenetRename(net(), name));
    });
    setRenameAction(action);
    setZValue(1);
}

gpds::container FancyWire::to_container() const
{
    // Root
    gpds::container root;
    addItemTypeIdToContainer(root);
    root.add_value("wire", QSchematic::Wire::to_container());

    return root;
}

void FancyWire::from_container(const gpds::container& container)
{
    QSchematic::Wire::from_container(*container.get_value<gpds::container*>("wire").value());
}

std::shared_ptr<QSchematic::Item> FancyWire::deepCopy() const
{
    auto clone = QSchematic::mk_sh<FancyWire>(parentItem());
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

    // Make points fancy if they are on top of one of our connectors
    painter->setPen(pen);
    painter->setBrush(brush);

    for (const auto& connector: scene()->connectors()) {
        if (connector->attachedWire() == this) {
            painter->drawEllipse(mapFromScene(connector->scenePos()), SIZE, SIZE);
        }
    }
}
