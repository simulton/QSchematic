#include <QJsonObject>
#include <QPainter>
#include "flowstart.h"
#include "itemtypes.h"
#include "operationconnector.h"
#include "../../../lib/items/label.h"

#define SIZE (_settings.gridSize/2)
#define RECT (QRectF(-SIZE, -SIZE, 2*SIZE, 2*SIZE))

FlowStart::FlowStart() :
    QSchematic::Node(::ItemType::FlowStartType)
{
    // Connector
    _connector = std::make_shared<OperationConnector>();
    _connector->setParentItem(this);
    _connector->label()->setVisible(false);
    _connector->label()->setMovable(false);
    _connector->setGridPos(0, 0);
    addConnector(_connector);

    // Label
    label()->setText("Start");
    label()->setVisible(true);

    // Misc
    setSize(2, 2);
    setAllowMouseResize(false);
}

QJsonObject FlowStart::toJson() const
{
    QJsonObject object;

    object.insert("node", QSchematic::Node::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool FlowStart::fromJson(const QJsonObject& object)
{
    QSchematic::Node::fromJson(object["node"].toObject());

    return true;
}

std::unique_ptr<QSchematic::Item> FlowStart::deepCopy() const
{
    auto clone = std::make_unique<FlowStart>();
    copyAttributes(*(clone.get()));

    return clone;
}

void FlowStart::copyAttributes(FlowStart& dest) const
{
    QSchematic::Node::copyAttributes(dest);
}

void FlowStart::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::gray);
    QRectF bodyRect(-settings().gridSize, -settings().gridSize, 2*settings().gridSize, 2*settings().gridSize);
    painter->drawEllipse(bodyRect);

    painter->setPen(Qt::red);
    painter->drawPoint(0, 0);
}
