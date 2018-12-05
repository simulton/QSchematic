#include <QPainter>
#include <QRectF>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QJsonObject>
#include "conditionconnector.h"
#include "itemtypes.h"

#define SIZE (_settings.gridSize/4)
#define RECT (QRectF(-SIZE, -SIZE, 2*SIZE, 2*SIZE))

const QColor COLOR_BODY_FILL   = QColor(Qt::black);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

ConditionConnector::ConditionConnector(const QPoint& gridPoint, const QString& text, QGraphicsItem* parent) :
    QSchematic::Connector(::ItemType::ConditionConnectorType, gridPoint, text, parent)
{
    setLabelIsVisible(false);
    setForceTextDirection(true);
    setForcedTextDirection(QSchematic::LeftToRight);
}

QJsonObject ConditionConnector::toJson() const
{
    QJsonObject object;

    object.insert("connector", QSchematic::Connector::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool ConditionConnector::fromJson(const QJsonObject& object)
{
    QSchematic::Connector::fromJson(object["connector"].toObject());

    return true;
}

std::unique_ptr<QSchematic::Item> ConditionConnector::deepCopy() const
{
    auto clone = std::make_unique<ConditionConnector>(gridPos(), text(), parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void ConditionConnector::copyAttributes(ConditionConnector& dest) const
{
    QSchematic::Connector::copyAttributes(dest);
}

QRectF ConditionConnector::boundingRect() const
{
    return RECT;
}

void ConditionConnector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

void ConditionConnector::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
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
