#include <QPainter>
#include <QRectF>
#include "myconnector.h"

#define SIZE (_settings.gridSize/4)
#define RECT (QRectF(-SIZE, -SIZE, 2*SIZE, 2*SIZE))

const QColor COLOR_BODY_FILL   = QColor(Qt::black);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

MyConnector::MyConnector(const QPoint& gridPoint, QGraphicsItem* parent) :
    QSchematic::Connector(gridPoint, QString(), parent)
{
    setLabelVisibility(false);
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
