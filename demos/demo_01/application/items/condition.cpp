#include <QPainter>
#include <QJsonObject>
#include "condition.h"
#include "conditionconnector.h"
#include "itemtypes.h"
#include "../../../lib/utils.h"

const QColor COLOR_HIGHLIGHTED = QColor(Qt::blue).lighter();
const QColor COLOR_BODY_FILL   = QColor(Qt::gray).lighter(140);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal SHAPE_PADDING      = 10;
const qreal PEN_WIDTH          = 1.5;

Condition::Condition(QGraphicsItem* parent) :
    QSchematic::Node(::ItemType::ConditionType, parent)
{
    setSize(10, 6);
    setMouseResizePolicy(static_cast<ResizePolicy>(QSchematic::Node::HorizontalEvenOnly | QSchematic::Node::VerticalEvenOnly));
    setAllowMouseResize(true);
    setConnectorsMovable(false);
    setConnectorsSnapPolicy(QSchematic::Connector::Anywhere);
    setConnectorsSnapToGrid(false);

    // Connections
    connect(this, &QSchematic::Node::sizeChanged, this, &Condition::placeConnectors);

    // Misc
    calculatePolygon();
    placeConnectors();
}

QJsonObject Condition::toJson() const
{
    QJsonObject object;

    object.insert("node", QSchematic::Node::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool Condition::fromJson(const QJsonObject& object)
{
    QSchematic::Node::fromJson(object["node"].toObject());

    // Clear connectors as we have fixed ones in this class
    update();

    return true;
}

QPainterPath Condition::shape() const
{
    QPainterPath basePath;
    basePath.addPolygon(_polygon);

    return basePath;
}

void Condition::update()
{
    calculatePolygon();
    placeConnectors();

    Node::update();
}

void Condition::placeConnectors()
{
    // Get rid of current connectors
    clearConnectors();

    // Add new connectors
    auto points = QSchematic::Utils::rectanglePoints(sizeRect(), QSchematic::Utils::RectangleEdgeCenterPoints);
    for (const auto& point : points) {
        addConnector(new ConditionConnector(point.toPoint(), QStringLiteral("[case]")));
    }
}

void Condition::calculatePolygon()
{
    auto points = QSchematic::Utils::rectanglePoints(sizeRect(), QSchematic::Utils::RectangleEdgeCenterPoints);
    auto scaledPoints(points);
    for (auto it = scaledPoints.begin() ; it != scaledPoints.end(); it++) {
        *it = *it * _settings.gridSize;
    }

    _polygon = QPolygonF(scaledPoints);
}

void Condition::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Draw the bounding rect if debug mode is enabled
    if (_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawRect(boundingRect());
    }

    // Highlight rectangle
    if (isHighlighted()) {
        // Highlight pen
        QPen highlightPen;
        highlightPen.setStyle(Qt::NoPen);

        // Highlight brush
        QBrush highlightBrush;
        highlightBrush.setStyle(Qt::SolidPattern);
        highlightBrush.setColor(COLOR_HIGHLIGHTED);

        // Highlight rectangle
        painter->setPen(highlightPen);
        painter->setBrush(highlightBrush);
        painter->setOpacity(0.5);
        painter->drawPath(shape());
    }

    painter->setOpacity(1.0);

    // Body pen
    QPen bodyPen;
    bodyPen.setWidthF(PEN_WIDTH);
    bodyPen.setStyle(Qt::SolidLine);
    bodyPen.setColor(COLOR_BODY_BORDER);

    // Body brush
    QBrush bodyBrush;
    bodyBrush.setStyle(Qt::SolidPattern);
    bodyBrush.setColor(COLOR_BODY_FILL);

    // Draw body
    painter->setPen(bodyPen);
    painter->setBrush(bodyBrush);
    painter->drawPolygon(_polygon);

    // Resize handles
    if (isSelected() and allowMouseResize()) {
        paintResizeHandles(*painter);
    }
}
