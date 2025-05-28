#include "bezierwire.hpp"

#include <QPainter>
#include <QPainterPathStroker>
#include <QtMath>

const int LINE_WIDTH              = 2;
const qreal HANDLE_SIZE           = 3.0;
const qreal CTRL_POINT_RATIO      = 0.5;
const QColor COLOR                = QColor("#000000");
const QColor COLOR_HIGHLIGHTED    = QColor("#dc2479");
const QColor COLOR_SELECTED       = QColor("#0f16af");

using namespace QSchematic::Items;

BezierWire::BezierWire(int type, QGraphicsItem* parent) :
    Wire(type, parent)
{
}

void BezierWire::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

QPainterPath BezierWire::path() const
{
    // Retrieve the scene points as we'll need them a lot
    auto sceneWirePoints(wirePointsRelative());
    QVector<point> scenePoints;
    for (const auto& wirePoint : sceneWirePoints) {
        scenePoints << wirePoint;
    }

    // Nothing to do if there are no points
    if (scenePoints.count() < 2) {
        return { };
    }

    QPainterPath path;
    path.moveTo(scenePoints.at(0).toPointF());

    for (int i = 0; i < scenePoints.count()-1; i++) {
        // Retrieve points
        const point p1 = scenePoints.at(i);
        const point p2 = scenePoints.at(i+1);

        // Calculate control points
        qreal dx = (p2.x() - p1.x()) * CTRL_POINT_RATIO;
        QPointF control1(p1.x() + dx, p1.y());
        QPointF control2(p2.x() - dx, p2.y());

        path.cubicTo(control1, control2, p2.toPointF());
    }

    return path;
}

QPainterPath BezierWire::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    stroker.setCapStyle(Qt::RoundCap);
    return stroker.createStroke(path());
}

QRectF BezierWire::boundingRect() const
{
    return shape().boundingRect();
}
