#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QVector2D>
#include "wirenetlabel.h"
#include "wirenet.h"
#include "../utils.h"

const QColor COLOR_WIRE             = QColor("#000000");
const QColor COLOR_WIRE_HIGHLIGHTED = QColor("#dc2479");

using namespace QSchematic;

WireNetLabel::WireNetLabel(WireNet& net) :
    _net(net)
{
}

void WireNetLabel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Draw a dashed line to the wire if selected
    if (isHighlighted()) {
        QPen penLine;
        penLine.setColor(COLOR_WIRE_HIGHLIGHTED);
        penLine.setStyle(Qt::DashLine);

        QBrush brushLine;
        brushLine.setStyle(Qt::NoBrush);

        // Find a point of the wire closest to our current location
        QPointF pointOnLine(INT_MAX, INT_MAX);
        for (const Line& line : _net.lineSegments()) {
            QPointF lineP1 = _settings.toScenePoint(line.p1());
            QPointF lineP2 = _settings.toScenePoint(line.p2());
            QPointF labelCenter = mapToScene(textRect().center());

            QPointF pointOnCurrentLine = Utils::pointOnLineClosestToPoint(lineP1, lineP2, labelCenter).toPoint();

            if (QVector2D(pointOnCurrentLine).distanceToPoint(QVector2D(labelCenter)) < QVector2D(pointOnLine).distanceToPoint(QVector2D(labelCenter))) {
                pointOnLine = pointOnCurrentLine;
            }
        }

        // Draw the connection line
        painter->setPen(penLine);
        painter->setBrush(brushLine);
        painter->drawLine(textRect().center(), mapFromParent(pointOnLine));

        // Clear the text rectangle
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(boundingRect().adjusted(penLine.width()/2, penLine.width()/2, -penLine.width()/2, -penLine.width()/2));

        // Draw the border around the label text
        painter->setPen(penLine);
        painter->setBrush(brushLine);
        painter->drawRect(textRect());
    }

    // Draw the text
    painter->setPen(COLOR_WIRE);
    painter->setBrush(Qt::NoBrush);
    painter->setFont(font());
    painter->drawText(textRect(), Qt::AlignCenter, text());
}
