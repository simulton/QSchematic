#include "wireroundedcorners.h"
#include "../utils.h"
#include "../wire_system/point.h"
#include "../wire_system/line.h"

#include <QPainter>
#include <QVector2D>

const qreal BOUNDING_RECT_PADDING = 6.0;
const qreal HANDLE_SIZE           = 3.0;
const qreal WIRE_SHAPE_PADDING    = 10;
const int LINE_WIDTH              = 2;
const QColor COLOR                = QColor("#000000");
const QColor COLOR_HIGHLIGHTED    = QColor("#dc2479");
const QColor COLOR_SELECTED       = QColor("#0f16af");

using namespace QSchematic;

WireRoundedCorners::WireRoundedCorners(int type, QGraphicsItem* parent) :
    Wire(type, parent)
{
}

gpds::container WireRoundedCorners::to_container() const
{
    // Root
    gpds::container root;
    root.add_value("wire", Wire::to_container());

    return root;
}

void WireRoundedCorners::from_container(const gpds::container& container)
{
    // Root
    auto opt = container.get_value<gpds::container*>("wire");
    Wire::from_container(opt ? *opt.value() : gpds::container());
}

void WireRoundedCorners::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Retrieve the scene points as we'll need them a lot
    auto sceneWirePoints(wirePointsRelative());
    QVector<point> scenePoints;
    for (const auto& wirePoint : sceneWirePoints) {
        scenePoints << wirePoint;
    }

    // Nothing to do if there are no points
    if (scenePoints.count() < 2) {
        return;
    }

    QPen penJunction;
    penJunction.setStyle(Qt::NoPen);

    QBrush brushJunction;
    brushJunction.setStyle(Qt::SolidPattern);
    brushJunction.setColor(isHighlighted() ? COLOR_HIGHLIGHTED : COLOR);

    // Draw the actual line
    {
        QPainterPath path;
        // Pen
        QPen penLine;
        penLine.setStyle(Qt::SolidLine);
        penLine.setCapStyle(Qt::RoundCap);
        QColor penColor;
        if (isSelected()) {
            penColor = COLOR_SELECTED;
        } else if (isHighlighted()) {
            penColor = COLOR_HIGHLIGHTED;
        } else {
            penColor = COLOR;
        }
        penLine.setWidth(LINE_WIDTH);
        penLine.setColor(penColor);

        // Brush
        QBrush brushLine;
        brushLine.setStyle(Qt::NoBrush);

        // Prepare the painter
        painter->setPen(penLine);
        painter->setBrush(brushLine);

        // Render
        for (int i = 0; i < scenePoints.count(); i++) {
            // Retrieve point
            point point = scenePoints.at(i);

            // If it's the last point
            if (i == scenePoints.count()-1) {
                path.lineTo(point.toPointF());
            }
            // If it's the first point
            else if (i == 0) {
                wire_system::point nPoint = scenePoints.at(i + 1);
                path.moveTo(point.toPointF());
                path.lineTo(Utils::centerPoint(point.toPointF(), nPoint.toPointF()));
            }
            // It's a point in the middle of the wire
            else {
                // Get the previous and next points
                wire_system::point pPoint = scenePoints.at(i - 1);
                wire_system::point nPoint = scenePoints.at(i + 1);

                // Find if there is a junction on this point
                bool hasJunction = false;
                for (const auto& wire: connected_wires()) {
                    for (const auto& jIndex: wire->junctions()) {
                        const auto& junction = wire->points().at(jIndex);
                        if (junction.toPoint() == (point + pos()).toPoint()) {
                            hasJunction = true;
                            break;
                        }
                    }
                    if (hasJunction) {
                        break;
                    }
                }

                // Lines form the current point up to half way to the next/previous point
                QLineF line1(Utils::centerPoint(pPoint.toPoint(), point.toPoint()), point.toPoint());
                QLineF line2(Utils::centerPoint(point.toPoint(), nPoint.toPoint()), point.toPoint());

                int linePointAdjust = _settings.gridSize/2;
                // If one of the lines is smaller that linePointAdjust make its length the new linePointAdjust
                if (line1.length() < linePointAdjust) {
                    linePointAdjust = line1.length();
                }
                if (line2.length() < linePointAdjust) {
                    linePointAdjust = line2.length();
                }
                // We certainly don't want an arc if this is a junction
                if (!hasJunction && !point.is_junction()) {
                    // Shorten lines if there is a rounded corner
                    line1.setLength(line1.length() - linePointAdjust);
                    line2.setLength(line2.length() - linePointAdjust);
                }

                // Render lines
                path.lineTo(line1.p2());
                // Render the arc if there is no junction
                if (!hasJunction && !point.is_junction()) {
                    path.quadTo(point.toPointF(), line2.p2());
                }
                path.lineTo(line2.p2());
            }
        }
        painter->drawPath(path);
    }

    // Draw the junction points
    int junctionRadius = 4;
    for (const wire_system::point& wirePoint : wirePointsRelative()) {
        if (wirePoint.is_junction()) {
            painter->setPen(penJunction);
            painter->setBrush(brushJunction);
            painter->drawEllipse(wirePoint.toPoint(), junctionRadius, junctionRadius);
        }
    }

    // Draw the handles (if selected)
    if (isSelected()) {
        // Pen
        QPen penHandle;
        penHandle.setColor(Qt::black);
        penHandle.setStyle(Qt::SolidLine);

        // Brush
        QBrush brushHandle;
        brushHandle.setColor(Qt::black);
        brushHandle.setStyle(Qt::SolidPattern);

        // Render
        painter->setPen(penHandle);
        painter->setBrush(brushHandle);
        for (const point& point : scenePoints) {
            QRectF handleRect(point.x() - HANDLE_SIZE, point.toPoint().y() - HANDLE_SIZE, 2*HANDLE_SIZE, 2*HANDLE_SIZE);
            painter->drawRect(handleRect);
        }
    }

    // Draw debugging stuff
    if (_settings.debug) {
        painter->setPen(Qt::red);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());

        painter->setPen(Qt::blue);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(shape());
    }
}
