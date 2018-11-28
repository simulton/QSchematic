#include <QPainter>
#include <QJsonObject>
#include "condition.h"
#include "myconnector.h"
#include "itemtypes.h"
#include "../../../lib/utils.h"

const QColor COLOR_HIGHLIGHTED = QColor(Qt::blue).lighter();
const QColor COLOR_BODY_FILL   = QColor(Qt::gray).lighter(140);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
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
    placeConnectors();
}

QJsonObject Condition::toJson() const
{
    QJsonObject object;

    object.insert("item", Item::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool Condition::fromJson(const QJsonObject& object)
{
    Item::fromJson(object["item"].toObject());

    return true;
}

void Condition::placeConnectors()
{
    // Get rid of current connectors
    clearConnectors();

    // Add new connectors
    auto points = QSchematic::Utils::rectanglePoints(sizeRect(), QSchematic::Utils::RectangleEdgeCenterPoints);
    for (const auto& point : points) {
        addConnector(new MyConnector(point.toPoint(), QStringLiteral("[case]")));
    }
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
        int adj = _settings.highlightRectPadding;
        painter->drawRoundedRect(QRect(QPoint(0, 0), size()*_settings.gridSize).adjusted(-adj, -adj, adj, adj), _settings.gridSize/2, _settings.gridSize/2);
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

    // Draw the component body
    {
        // Create polygon
        auto points = QSchematic::Utils::rectanglePoints(sizeRect(), QSchematic::Utils::RectangleEdgeCenterPoints);
        auto scaledPoints(points);
        for (auto it = scaledPoints.begin() ; it != scaledPoints.end(); it++) {
            *it = *it * _settings.gridSize;
        }

        // Draw
        painter->setPen(bodyPen);
        painter->setBrush(bodyBrush);
        painter->drawPolygon(scaledPoints.constData(), scaledPoints.count());
    }

    // Resize handles
    if (isSelected() and allowMouseResize()) {
        for (const QRect& rect : resizeHandles()) {
            // Handle pen
            QPen handlePen;
            handlePen.setStyle(Qt::NoPen);
            painter->setPen(handlePen);

            // Handle Brush
            QBrush handleBrush;
            handleBrush.setStyle(Qt::SolidPattern);
            painter->setBrush(handleBrush);

            // Draw the outer handle
            handleBrush.setColor("#3fa9f5");
            painter->setBrush(handleBrush);
            painter->drawRect(rect.adjusted(-handlePen.width(), -handlePen.width(), handlePen.width()/2, handlePen.width()/2));

            // Draw the inner handle
            int adj = _settings.resizeHandleSize/2;
            handleBrush.setColor(Qt::white);
            painter->setBrush(handleBrush);
            painter->drawRect(rect.adjusted(-handlePen.width()+adj, -handlePen.width()+adj, (handlePen.width()/2)-adj, (handlePen.width()/2)-adj));
        }
    }
}
