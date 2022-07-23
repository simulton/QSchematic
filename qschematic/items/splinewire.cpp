#include "splinewire.h"

#include <QPainter>
#include <QPainterPathStroker>
#include <QtMath>

const int LINE_WIDTH              = 2;
const qreal HANDLE_SIZE           = 3.0;
const qreal CTRL_POINT_RATIO      = 0.3;
const QColor COLOR                = QColor("#000000");
const QColor COLOR_HIGHLIGHTED    = QColor("#dc2479");
const QColor COLOR_SELECTED       = QColor("#0f16af");

using namespace QSchematic;

SplineWire::SplineWire(int type, QGraphicsItem* parent) :
    Wire(type, parent)
{

}

void SplineWire::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

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

    painter->drawPath(path());

    // Draw the junction poins
    QPen penJunction;
    penJunction.setStyle(Qt::NoPen);

    QBrush brushJunction;
    brushJunction.setStyle(Qt::SolidPattern);
    brushJunction.setColor(isHighlighted() ? COLOR_HIGHLIGHTED : COLOR);

    int junctionRadius = 4;
    for (const point& wirePoint : wirePointsRelative()) {
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
        for (const point& point : wirePointsRelative()) {
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

QPainterPath SplineWire::path() const
{
    QPainterPath path;

    // Retrieve the scene points as we'll need them a lot
    auto sceneWirePoints(wirePointsRelative());
    QVector<point> scenePoints;
    for (const auto& wirePoint : sceneWirePoints) {
        scenePoints << wirePoint;
    }

    // Nothing to do if there are no points
    if (scenePoints.count() < 2) {
        return QPainterPath();
    }

    // Render
    for (int i = 0; i <= scenePoints.count()-2; i++) {
        // Retrieve points
        point p1 = scenePoints.at(i);
        point p2 = scenePoints.at(i + 1);

        // If there are just two points we need to render the line just like this and we're done
        if (scenePoints.count() == 2) {
            path.moveTo(p1.toPointF());
            path.lineTo(p2.toPointF());
            break;

        // If we have two line segments we want to render an arc connecting them
        } else if (i < scenePoints.count()-2) {
            point p3 = scenePoints.at(i + 2);

            // Tangent at p2
            QPointF ctrlP1;
            {
                const auto d1 = p2.toPointF() - p1.toPointF();
                const auto d2 = p2.toPointF() - p3.toPointF();
                qreal angle1 = qAtan2(d1.x(), d1.y());
                qreal angle2 = qAtan2(d2.x(), d2.y());
                qreal alpha = M_PI - (angle1 + angle2)/2;
                // Flip it when both vectors are aligned
                if (angle2 > angle1 && (angle2 > 0 || angle1 < 0)) {
                    alpha += M_PI;
                }
                ctrlP1 = QPointF(-d2.manhattanLength()*CTRL_POINT_RATIO * qCos(alpha) + p2.x(), -d2.manhattanLength()*CTRL_POINT_RATIO * qSin(alpha) + p2.y());

                if (i == 0) {
                    QPointF ctrlP(d1.manhattanLength()*CTRL_POINT_RATIO * qCos(alpha) + p2.x(), d1.manhattanLength()*CTRL_POINT_RATIO * qSin(alpha) + p2.y());
                    path.moveTo(p1.toPointF());
                    path.quadTo(ctrlP, p2.toPointF());
                } else {
                    path.moveTo(p2.toPointF());
                }
            }

            if (i < scenePoints.count()-3) {
                point p4 = scenePoints.at(i + 3);

                // Tangent at p3
                QPointF ctrlP2;
                {
                    const auto d1 = p3.toPointF() - p2.toPointF();
                    const auto d2 = p3.toPointF() - p4.toPointF();
                    qreal angle1 = qAtan2(d1.x(), d1.y());
                    qreal angle2 = qAtan2(d2.x(), d2.y());
                    qreal beta = M_PI - (angle1 + angle2)/2;
                    // Flip it when both vectors are aligned
                    if (angle2 > angle1 && (angle2 > 0 || angle1 < 0)) {
                        beta += M_PI;
                    }
                    ctrlP2 = QPointF(d1.manhattanLength()*CTRL_POINT_RATIO * qCos(beta) + p3.x(), d1.manhattanLength()*CTRL_POINT_RATIO * qSin(beta) + p3.y());
                }
                path.cubicTo(ctrlP1, ctrlP2, p3.toPointF());
            } else {
                path.quadTo(ctrlP1, p3.toPointF());
            }
        }
    }
    return path;
}

QPainterPath SplineWire::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    stroker.setCapStyle(Qt::RoundCap);
    return stroker.createStroke(path());
}

QRectF SplineWire::boundingRect() const
{
    return shape().boundingRect();
}
