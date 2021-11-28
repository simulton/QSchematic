#include <QPainter>
#include <QGraphicsDropShadowEffect>

#include "flowstart.h"
#include "itemtypes.h"
#include "operationconnector.h"
#include "qschematic/items/label.h"

const QColor COLOR_BODY_FILL   = QColor( QStringLiteral( "#e0e0e0" ) );
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
    auto connector = std::make_shared<OperationConnector>();
    connector->setParentItem(this);
    connector->label()->setVisible(false);
    connector->label()->setMovable(false);
    connector->setGridPos(1, 0);
    addConnector(connector);

    // Drop shadow
    auto graphicsEffect = new QGraphicsDropShadowEffect(this);
    graphicsEffect->setOffset(SHADOW_OFFSET);
    graphicsEffect->setBlurRadius(SHADOW_BLUR_RADIUS);
    graphicsEffect->setColor(SHADOW_COLOR);
    setGraphicsEffect(graphicsEffect);

    // Misc
    setSize(60, 40);
    setAllowMouseResize(false);
}

gpds::container FlowStart::to_container() const
{
    // Root
    gpds::container root;
    addItemTypeIdToContainer(root);
    root.add_value("node", QSchematic::Node::to_container());

    return root;
}

void FlowStart::from_container(const gpds::container& container)
{
    QSchematic::Node::from_container(*container.get_value<gpds::container*>("node").value());
}

std::shared_ptr<QSchematic::Item> FlowStart::deepCopy() const
{
    auto clone = std::make_shared<FlowStart>();
    copyAttributes(*clone);

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
