#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QMetaEnum>
#include <QVector2D>
#include "wire.h"
#include "../utils.h"

const qreal BOUNDING_RECT_PADDING = 6.0;
const qreal HANDLE_SIZE = 3.0;
const qreal WIRE_SHAPE_PADDING = 10;
const QColor COLOR                     = QColor("#000000");
const QColor COLOR_HIGHLIGHTED         = QColor("#dc2479");
const QColor COLOR_SELECTED            = QColor("#0f16af");

using namespace QSchematic;

class PointWithIndex {
public:
    PointWithIndex(int index, const QPoint& point) : index(index), point(point) {}
    int index;
    QPoint point;

    bool operator<(const PointWithIndex& other) const {
        return index < other.index;
    }
};

Wire::Wire(int type, QGraphicsItem* parent) :
    Item(type, parent)
{
    _pointToMoveIndex = -1;
    _lineSegmentToMoveIndex = -1;

    // Lines should always be the lowest item in Z-Order
    setZValue(-10);

    // ALWAYS snap to grid
    setSnapToGrid(true);
}

QJsonObject Wire::toJson() const
{
    QJsonObject object;

    // Points
    QJsonArray pointsArray;
    for (int i = 0; i < _points.count(); i++) {
        QJsonObject pointObject;
        pointObject.insert("index", i);
        pointObject.insert("x", _points.at(i).x());
        pointObject.insert("y", _points.at(i).y());

        pointsArray.append(pointObject);
    }
    object.insert("points", pointsArray);

    // Base class
    object.insert("item", Item::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool Wire::fromJson(const QJsonObject& object)
{
    Item::fromJson(object["item"].toObject());

    // Points array
    QList<PointWithIndex> pointsUnsorted;
    QJsonArray pointsArray = object["points"].toArray();
    if (!pointsArray.isEmpty()) {
        for (QJsonValue pointsValue : pointsArray) {
            QJsonObject pointsObject = pointsValue.toObject();
            if (pointsObject.isEmpty()) {
                continue;
            }

            pointsUnsorted.append(PointWithIndex(pointsObject["index"].toInt(), QPoint(pointsObject["x"].toInt(), pointsObject["y"].toInt())));
        }
    }
    std::sort(pointsUnsorted.begin(), pointsUnsorted.end());
    for (const PointWithIndex& pointWithIndex : pointsUnsorted) {
        _points.append(pointWithIndex.point);
    }

    // Update stuff
    update();

    return true;
}

void Wire::update()
{
    calculateBoundingRect();

    Item::update();
}

QRectF Wire::boundingRect() const
{
    return _rect.adjusted(-BOUNDING_RECT_PADDING, -BOUNDING_RECT_PADDING, BOUNDING_RECT_PADDING, BOUNDING_RECT_PADDING);
}

QPainterPath Wire::shape() const
{
    QPainterPath basePath;
    basePath.addPolygon(QPolygon(scenePointsRelative()));

    QPainterPathStroker str;
    str.setCapStyle(Qt::FlatCap);
    str.setJoinStyle(Qt::MiterJoin);
    str.setWidth(WIRE_SHAPE_PADDING);

    QPainterPath resultPath = str.createStroke(basePath).simplified();

    return resultPath;
}

QVector<WirePoint> Wire::sceneWirePointsRelative() const
{
    QVector<WirePoint> points;

    for (const WirePoint& point : _points) {
        WirePoint&& tmp = WirePoint(_settings.toScenePoint(point.toPoint()));
        tmp.setIsJunction(point.isJunction());
        points.append(tmp);
    }

    return points;
}

QVector<QPoint> Wire::scenePointsRelative() const
{
    QVector<QPoint> points;

    for (const WirePoint& point : _points) {
        points << _settings.toScenePoint(point.toPoint());
    }

    return points;
}

QVector<QPoint> Wire::scenePointsAbsolute() const
{
    QVector<QPoint> points;

    for (const WirePoint& point : _points) {
        points << _settings.toScenePoint(point.toPoint() + gridPoint());
    }

    return points;
}

void Wire::calculateBoundingRect()
{
    // Find top-left most point
    QPointF topLeft(INT_MAX, INT_MAX);
    for (auto& point : _points) {
        if (point.x() < topLeft.x())
            topLeft.setX(point.x());
        if (point.y() < topLeft.y())
            topLeft.setY(point.y());
    }

    // Find bottom-right most point
    QPointF bottomRight(INT_MIN, INT_MIN);
    for (auto& point : _points) {
        if (point.x() > bottomRight.x())
            bottomRight.setX(point.x());
        if (point.y() > bottomRight.y())
            bottomRight.setY(point.y());
    }

    // Create the rectangle
    _rect = QRectF(topLeft * _settings.gridSize, bottomRight * _settings.gridSize);
}

void Wire::prependPoint(const QPoint& point)
{
    prepareGeometryChange();
    _points.prepend(WirePoint(point - gridPoint()));
    calculateBoundingRect();

    emit pointMoved(*this, _points.first());
}

void Wire::appendPoint(const QPoint& point)
{
    prepareGeometryChange();
    _points.append(WirePoint(point - gridPoint()));
    calculateBoundingRect();

    emit pointMoved(*this, _points.last());
}

void Wire::insertPoint(int index, const QPoint& point)
{
    // Boundary check
    if (index < 0 || index >= _points.count()) {
        return;
    }

    prepareGeometryChange();
    _points.insert(index, WirePoint(point - gridPoint()));
    calculateBoundingRect();

    emit pointMoved(*this, _points[index]);
}

void Wire::removeFirstPoint()
{
    if (_points.count() <= 0) {
        return;
    }
    prepareGeometryChange();
    _points.removeFirst();
    calculateBoundingRect();
}

void Wire::removeLastPoint()
{
    if (_points.count() <= 0) {
        return;
    }

    prepareGeometryChange();
    _points.removeLast();
    calculateBoundingRect();
}

void Wire::removePoint(const QPoint& point)
{
    prepareGeometryChange();
    _points.removeAll(WirePoint(point - gridPoint()));
    calculateBoundingRect();
}

void Wire::simplify()
{
    removeDuplicatePoints();
    removeObsoletePoints();
}

void Wire::removeDuplicatePoints()
{
    QVector<WirePoint> newList;

    for (auto it = _points.begin(); it != _points.constEnd(); it++) {
        if (!newList.contains(*it)) {
            newList << *it;
        }
    }

    _points = newList;
}

void Wire::removeObsoletePoints()
{
    // Don't do anything if there are not at least three line segments
    if (_points.count() < 3) {
        return;
    }

    // Compile a list of obsolete points
    QList<WirePoint> pointsToRemove;
    for (int i = 2; i < _points.count(); i++) {
        QPoint p1 = _points.at(i - 2).toPoint();
        QPoint p2 = _points.at(i - 1).toPoint();
        QPoint p3 = _points.at(i).toPoint();

        // Check if p2 is on the line created by p1 and p3
        if (Utils::pointIsOnLine(QLineF(p1, p2), p3)) {
            pointsToRemove.append(_points[i-1]);
        }
    }

    // Get rid of them
    for (const WirePoint& point : pointsToRemove) {
        removePoint(point.toPoint());
    }
}

void Wire::movePointBy(int index, const QVector2D& moveBy)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    prepareGeometryChange();
    _points[index] = WirePoint(_points.at(index).toPoint() + moveBy.toPoint());
    calculateBoundingRect();
    update();

    emit pointMoved(*this, _points[index]);
}

void Wire::movePointTo(int index, const QPoint& moveTo)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    prepareGeometryChange();
    _points[index] = WirePoint(moveTo - gridPoint());
    calculateBoundingRect();    
    update();

    emit pointMoved(*this, _points[index]);
}

void Wire::moveLineSegmentBy(int index, const QVector2D& moveBy)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    // Move the line segment
    movePointBy(index, moveBy);
    movePointBy(index+1, moveBy);
}

void Wire::setPointIsJunction(int index, bool isJunction)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    _points[index].setIsJunction(isJunction);

    update();
}

bool Wire::pointIsOnWire(const QPoint& point) const
{
    for (const Line& lineSegment : lineSegments()) {
        if (lineSegment.containsPoint(point, 0)) {
            return true;
        }
    }

    return false;
}

QVector<QPoint> Wire::points() const
{
    QVector<QPoint> list;

    for (const WirePoint& wirePoint : _points) {
        list << gridPoint() + wirePoint.toPoint();
    }

    return list;
}

QList<Line> Wire::lineSegments() const
{
    // A line segment requires at least two points... duuuh
    if (_points.count() < 2) {
        return QList<Line>();
    }

    QList<Line> ret;
    for (int i = 0; i < _points.count()-1; i++) {
        ret.append(Line(gridPoint() + _points.at(i).toPoint(), gridPoint() + _points.at(i+1).toPoint()));
    }

    return ret;
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    _prevMousePos = _settings.toGridPoint(event->scenePos());

    // Check wheter we clicked on a handle
    if (isSelected()) {
        // Check whether we clicked on a handle
        QVector<QPoint> points(scenePointsAbsolute());
        for (int i = 0; i < points.count(); i++) {
            QRectF handleRect(points.at(i).x() - HANDLE_SIZE, points.at(i).y() - HANDLE_SIZE, 2*HANDLE_SIZE, 2*HANDLE_SIZE);

            if (handleRect.contains(event->scenePos())) {
                _pointToMoveIndex = i;
                return;
            }

            _pointToMoveIndex = -1;
        }

        // Check whether we clicked on a line segment
        QList<Line> lines = lineSegments();
        for (int i = 0; i < lines.count(); i++) {
            const Line& line = lines.at(i);
            if (line.containsPoint(_settings.toGridPoint(event->scenePos()), 1)) {
                _lineSegmentToMoveIndex = i;
                return;
            }

            _lineSegmentToMoveIndex = -1;
        }

    } else {
        Item::mousePressEvent(event);
    }
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Item::mouseReleaseEvent(event);

    _pointToMoveIndex = -1;
    _lineSegmentToMoveIndex = -1;
    _prevMousePos = _settings.toGridPoint(event->scenePos());
}

void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QPoint curPos = _settings.toGridPoint(event->scenePos());
    bool ctrlPressed = QApplication::keyboardModifiers() & Qt::ControlModifier;

    if (_pointToMoveIndex > -1) {

        event->accept();
        movePointTo(_pointToMoveIndex, curPos);

    } else if (_lineSegmentToMoveIndex > -1){

        event->accept();
        Line line = lineSegments().at(_lineSegmentToMoveIndex);
        QVector2D moveLineBy(0, 0);
        if (line.isHorizontal()) {
            moveLineBy = QVector2D(0, curPos.y() - _prevMousePos.y());
        } else if (line.isVertical()) {
            moveLineBy = QVector2D(curPos.x() - _prevMousePos.x(), 0);
        } else if (ctrlPressed){
            moveLineBy = QVector2D(curPos - _prevMousePos);
        }
        moveLineSegmentBy(_lineSegmentToMoveIndex, moveLineBy);

    } else {

        Item::mouseMoveEvent(event);
    }

    _prevMousePos = curPos;
}

void Wire::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Item::hoverEnterEvent(event);
}

void Wire::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Item::hoverLeaveEvent(event);

    unsetCursor();
}

void Wire::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    Item::hoverMoveEvent(event);

    // Only if wire is selected
    if (!isSelected()) {
        return;
    }

    // Check whether we hover over a point handle
    QVector<QPoint> points(scenePointsAbsolute());
    for (int i = 0; i < points.count(); i++) {
        QRectF handleRect(points.at(i).x() - HANDLE_SIZE, points.at(i).y() - HANDLE_SIZE, 2*HANDLE_SIZE, 2*HANDLE_SIZE);

        if (handleRect.contains(event->scenePos())) {
            setCursor(Qt::SizeAllCursor);
            return;
        }
    }

    // Check whether we hover over a line segment
    bool ctrlPressed = QApplication::keyboardModifiers() & Qt::ControlModifier;
    QList<Line> lines = lineSegments();
    for (int i = 0; i < lines.count(); i++) {
        const Line& line = lines.at(i);
        if (line.containsPoint(_settings.toGridPoint(event->scenePos()), 1)) {
            if (line.isHorizontal()) {
                setCursor(Qt::SizeVerCursor);
            } else if (line.isVertical()) {
                setCursor(Qt::SizeHorCursor);
            } else if (ctrlPressed) {
                setCursor(Qt::SizeAllCursor);
            }
            return;
        }
    }

    unsetCursor();
}

void Wire::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // ToDo: Merge the swich() statements into one
    Q_UNUSED(option);
    Q_UNUSED(widget);

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
    penLine.setWidth(1);
    penLine.setColor(penColor);

    QBrush brushLine;
    brushLine.setStyle(Qt::NoBrush);

    QPen penJunction;
    penJunction.setStyle(Qt::NoPen);

    QBrush brushJunction;
    brushJunction.setStyle(Qt::SolidPattern);
    brushJunction.setColor(isHighlighted() ? COLOR_HIGHLIGHTED : COLOR);

    QPen penHandle;
    penHandle.setColor(Qt::black);
    penHandle.setStyle(Qt::SolidLine);

    QBrush brushHandle;
    brushHandle.setColor(Qt::black);
    brushHandle.setStyle(Qt::SolidPattern);

    // Draw the actual line
    painter->setPen(penLine);
    painter->setBrush(brushLine);
    QVector<QPoint> points = scenePointsRelative();
    painter->drawPolyline(points.constData(), points.count());

    // Draw the junction poins
    int junctionRadius = 4;
    for (const WirePoint& wirePoint : sceneWirePointsRelative()) {
        if (wirePoint.isJunction()) {
            painter->setPen(penJunction);
            painter->setBrush(brushJunction);
            painter->drawEllipse(wirePoint.toPoint(), junctionRadius, junctionRadius);
        }
    }

    // Draw the handles (if selected)
    if (isSelected()) {
        painter->setOpacity(0.5);
        painter->setPen(penHandle);
        painter->setBrush(brushHandle);
        for (const QPoint& point : points) {
            QRectF handleRect(point.x() - HANDLE_SIZE, point.y() - HANDLE_SIZE, 2*HANDLE_SIZE, 2*HANDLE_SIZE);
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
