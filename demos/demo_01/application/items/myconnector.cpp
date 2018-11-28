#include <QPainter>
#include <QRectF>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QJsonObject>
#include "myconnector.h"

#define SIZE (_settings.gridSize/4)
#define RECT (QRectF(-SIZE, -SIZE, 2*SIZE, 2*SIZE))

const QColor COLOR_BODY_FILL   = QColor(Qt::black);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

MyConnector::MyConnector(const QPoint& gridPoint, const QString& text, QGraphicsItem* parent) :
    QSchematic::Connector(gridPoint, text, parent)
{
    setLabelIsVisible(false);
    setForceTextDirection(true);
    setForcedTextDirection(QSchematic::LeftToRight);
}

QJsonObject MyConnector::toJson() const
{
    QJsonObject object;

    object.insert("item", Item::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool MyConnector::fromJson(const QJsonObject& object)
{
    Item::fromJson(object["item"].toObject());

    return true;
}

QRectF MyConnector::boundingRect() const
{
    return RECT;
}

void MyConnector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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
    painter->drawRoundedRect(RECT, _settings.gridSize/6, _settings.gridSize/6);
}

void MyConnector::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
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
