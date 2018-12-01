#include <QPainter>
#include <QRectF>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QJsonObject>
#include "operationconnector.h"
#include "itemtypes.h"

#define SIZE (_settings.gridSize/2)
#define RECT (QRectF(-SIZE, -SIZE, 2*SIZE, 2*SIZE))

const QColor COLOR_BODY_FILL   = QColor(Qt::white);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

OperationConnector::OperationConnector(const QPoint& gridPoint, const QString& text, QGraphicsItem* parent) :
    QSchematic::Connector(::ItemType::OperationConnectorType, gridPoint, text, parent)
{
    setLabelIsVisible(false);
    setForceTextDirection(true);
    setForcedTextDirection(QSchematic::LeftToRight);
}

QJsonObject OperationConnector::toJson() const
{
    QJsonObject object;

    object.insert("connector", QSchematic::Connector::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool OperationConnector::fromJson(const QJsonObject& object)
{
    QSchematic::Connector::fromJson(object["connector"].toObject());

    return true;
}

QRectF OperationConnector::boundingRect() const
{
    return RECT;
}

void OperationConnector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

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
    painter->setPen(bodyPen);
    painter->setBrush(bodyBrush);
    painter->drawEllipse(RECT);
}

void OperationConnector::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Create the menu
    QMenu menu;
    {
        // Label visibility
        QAction* _labelVisibility = new QAction;
        _labelVisibility->setCheckable(true);
        _labelVisibility->setChecked(labelIsVisible());
        _labelVisibility->setText("Label visible");
        connect(_labelVisibility, &QAction::toggled, [this](bool enabled) {
            setLabelIsVisible(enabled);
        });

        // Assemble
        menu.addAction(_labelVisibility);
    }

    // Sow the menu
    menu.exec(event->screenPos());
}
