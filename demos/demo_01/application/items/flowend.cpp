#include <QJsonObject>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "flowend.h"
#include "itemtypes.h"
#include "operationconnector.h"
#include "../../../lib/items/label.h"

const QColor COLOR_BODY_FILL   = QColor(Qt::gray).lighter(140);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const QColor SHADOW_COLOR      = QColor(63, 63, 63, 100);
const qreal PEN_WIDTH          = 1.5;
const qreal SHADOW_OFFSET      = 7;
const qreal SHADOW_BLUR_RADIUS = 10;

FlowEnd::FlowEnd() :
    QSchematic::Node(::ItemType::FlowEndType)
{
    const int sz = _settings.gridSize;

    // Symbol polygon
    _symbolPolygon << QPoint(1*sz, -1*sz);
    _symbolPolygon << QPoint(0*sz, 0*sz);
    _symbolPolygon << QPoint(1*sz, 1*sz);
    _symbolPolygon << QPoint(3*sz, 1*sz);
    _symbolPolygon << QPoint(3*sz, -1*sz);

    // Connector
    _connector = std::make_shared<OperationConnector>();
    _connector->setParentItem(this);
    _connector->label()->setVisible(false);
    _connector->label()->setMovable(false);
    _connector->setGridPosX(1);
    _connector->setGridPosY(1);
    addConnector(_connector);

    // Label
    label()->setText("End");
    label()->setVisible(true);
    label()->setPos(1.0 * sz, 3.5 * sz);

    // Drop shadow
    auto graphicsEffect = new QGraphicsDropShadowEffect(this);
    graphicsEffect->setOffset(SHADOW_OFFSET);
    graphicsEffect->setBlurRadius(SHADOW_BLUR_RADIUS);
    graphicsEffect->setColor(SHADOW_COLOR);
    setGraphicsEffect(graphicsEffect);

    // Misc
    setSize(3, 2);
    setAllowMouseResize(false);
}

QJsonObject FlowEnd::toJson() const
{
    QJsonObject object;

    object.insert("node", QSchematic::Node::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool FlowEnd::fromJson(const QJsonObject& object)
{
    QSchematic::Node::fromJson(object["node"].toObject());

    return true;
}

std::unique_ptr<QSchematic::Item> FlowEnd::deepCopy() const
{
    auto clone = std::make_unique<FlowEnd>();
    copyAttributes(*(clone.get()));

    return clone;
}

void FlowEnd::copyAttributes(FlowEnd& dest) const
{
    QSchematic::Node::copyAttributes(dest);
}

QRectF FlowEnd::boundingRect() const
{
    return _symbolPolygon.boundingRect();
}

void FlowEnd::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Symbol
    {
        QPen pen(Qt::SolidLine);
        pen.setWidthF(PEN_WIDTH);
        pen.setColor(COLOR_BODY_BORDER);

        QBrush brush(Qt::SolidPattern);
        brush.setColor(COLOR_BODY_FILL);

        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawPolygon(_symbolPolygon);
    }
}
