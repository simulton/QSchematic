#include <QJsonObject>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "flowstart.h"
#include "itemtypes.h"
#include "operationconnector.h"
#include "../../../lib/items/label.h"

const QColor COLOR_BODY_FILL   = QColor(Qt::gray).lighter(140);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const QColor SHADOW_COLOR      = QColor(63, 63, 63, 100);
const qreal PEN_WIDTH          = 1.5;
const qreal SHADOW_OFFSET      = 7;
const qreal SHADOW_BLUR_RADIUS = 10;

FlowStart::FlowStart() :
    QSchematic::Node(::ItemType::FlowStartType)
{
    const int sz = _settings.gridSize;

    // Symbol polygon
    _symbolPolygon << QPoint(1*sz, 1*sz);
    _symbolPolygon << QPoint(2*sz, 0*sz);
    _symbolPolygon << QPoint(1*sz, -1*sz);
    _symbolPolygon << QPoint(-1*sz, -1*sz);
    _symbolPolygon << QPoint(-1*sz, 1*sz);

    // Connector
    _connector = std::make_shared<OperationConnector>();
    _connector->setParentItem(this);
    _connector->label()->setVisible(false);
    _connector->label()->setMovable(false);
    _connector->setGridPos(1, 0);
    addConnector(_connector);

    // Label
    label()->setText("Start");
    label()->setVisible(true);
    label()->setPos(-1.0 * sz, 2.5 * sz);

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

QRectF FlowStart::boundingRect() const
{
    QRectF rect = _symbolPolygon.boundingRect();
    qreal adj = SHADOW_BLUR_RADIUS;

    return rect.adjusted(-adj, -adj, adj, adj);
}

void FlowStart::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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
