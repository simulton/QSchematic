#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QMap>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QMetaEnum>
#include <QVector2D>
#include <QtMath>
#include <QMenu>
#include "wire.h"
#include "connector.h"
#include "scene.h"
#include "label.h"
#include "node.h"
#include "../utils.h"
#include "../commands/commandwirepointmove.h"

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
    Item(type, parent), _renameAction(nullptr), _internalMove(false)
{
    _pointToMoveIndex = -1;
    _lineSegmentToMoveIndex = -1;

    // Lines should always be the lowest item in Z-Order
    setZValue(-10);

    // ALWAYS snap to grid
    setSnapToGrid(true);
    setMovable(true);
}

gpds::container Wire::to_container() const
{
    // Points
    gpds::container pointsContainer;
    for (int i = 0; i < _points.count(); i++) {
        gpds::container pointContainer;
        pointContainer.add_attribute("index", i);
        pointContainer.add_value("x", _points.at(i).x());
        pointContainer.add_value("y", _points.at(i).y());
        pointsContainer.add_value("point", pointContainer);
    }

    // Root
    gpds::container rootContainer;
    addItemTypeIdToContainer(rootContainer);
    rootContainer.add_value("item", Item::to_container());
    rootContainer.add_value("points", pointsContainer);

    return rootContainer;
}

void Wire::from_container(const gpds::container& container)
{
    // Root
    Item::from_container(*container.get_value<gpds::container*>("item").value());

    // Points
    const gpds::container* pointsContainer = container.get_value<gpds::container*>("points").value_or(nullptr);
    if (pointsContainer) {
        auto points = pointsContainer->get_values<gpds::container*>("point");
        // Sort points by index
        std::sort(points.begin(), points.end(), [](gpds::container* a, gpds::container* b) {
            std::optional<int> index1 = a->get_attribute<int>("index");
            std::optional<int> index2 = b->get_attribute<int>("index");
            if (!index1.has_value() || !index2.has_value()) {
                qCritical("Wire::from_container(): Point has no index.");
                return false;
            }
            return index1.value() < index2.value();
        });
        for (const gpds::container* pointContainer : points ) {
            _points.append(WirePoint(pointContainer->get_value<double>("x").value_or(0),
                                     pointContainer->get_value<double>("y").value_or(0)));
        }
    }

    update();
}

std::shared_ptr<Item> Wire::deepCopy() const
{
    auto clone = mk_sh<Wire>(type(), parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void Wire::copyAttributes(Wire& dest) const
{
    Item::copyAttributes(dest);

    dest._points = _points;
    dest._rect = _rect;
    dest._pointToMoveIndex = _pointToMoveIndex;
    dest._lineSegmentToMoveIndex = _lineSegmentToMoveIndex;
    dest._prevMousePos = _prevMousePos;
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
    basePath.addPolygon(QPolygonF(pointsRelative()));

    QPainterPathStroker str;
    str.setCapStyle(Qt::FlatCap);
    str.setJoinStyle(Qt::MiterJoin);
    str.setWidth(WIRE_SHAPE_PADDING);

    QPainterPath resultPath = str.createStroke(basePath).simplified();

    return resultPath;
}

QVector<WirePoint> Wire::wirePointsRelative() const
{
    return _points;
}

QVector<WirePoint> Wire::wirePointsAbsolute() const
{
    QVector<WirePoint> absolutePoints(_points);

    for (WirePoint& point : absolutePoints) {
        bool isJunction = point.isJunction();
        point = point + pos();
        point.setIsJunction(isJunction);
    }

    return absolutePoints;
}

QVector<QPointF> Wire::pointsRelative() const
{
    QVector<QPointF> points;

    for (const WirePoint& point : _points) {
        points << point.toPointF();
    }

    return points;
}

QVector<QPointF> Wire::pointsAbsolute() const
{
    QVector<QPointF> points;

    for (const WirePoint& point : _points) {
        points << point + pos();
    }

    return points;
}

void Wire::calculateBoundingRect()
{
    // Find top-left most point
    const int& intMaxValue = std::numeric_limits<int>::max();
    QPointF topLeft(intMaxValue, intMaxValue);
    for (auto& point : _points) {
        if (point.x() < topLeft.x())
            topLeft.setX(point.x());
        if (point.y() < topLeft.y())
            topLeft.setY(point.y());
    }

    // Find bottom-right most point
    const int& intMinValue = std::numeric_limits<int>::min();
    QPointF bottomRight(intMinValue, intMinValue);
    for (auto& point : _points) {
        if (point.x() > bottomRight.x())
            bottomRight.setX(point.x());
        if (point.y() > bottomRight.y())
            bottomRight.setY(point.y());
    }

    // Create the rectangle
    _rect = QRectF(topLeft, bottomRight);
    if (movingWirePoint() and not topLeft.isNull()) {
        updatePosition();
    }
}

void Wire::updatePosition()
{
    QPointF topLeft = _rect.topLeft();
    for (int i = 0; i < _points.count(); i++) {
        _points[i].setX(_points[i].x() - topLeft.x());
        _points[i].setY(_points[i].y() - topLeft.y());
    }
    // Move all the child items
    for (auto& item : childItems()) {
        item->setPos(item->pos() - topLeft);
    }
    QPointF newPos = pos() + topLeft;
    QPointF snappedPos = _settings.snapToGrid(newPos);
    _offset = newPos - snappedPos;
    _internalMove = true;
    setPos(newPos);
    _internalMove = false;
    calculateBoundingRect();
}

void Wire::setRenameAction(QAction* action)
{
    _renameAction = action;
}

void Wire::prependPoint(const QPointF& point)
{
    prepareGeometryChange();
    _points.prepend(WirePoint(point - pos()));
    calculateBoundingRect();

    // Update junction
    if (_points.count() >= 2) {
        setPointIsJunction(0, _points.at(1).isJunction());
        setPointIsJunction(1, false);
    }

    emit pointInserted(0);
    emit pointMoved(*this, _points.first());
}

void Wire::appendPoint(const QPointF& point)
{
    prepareGeometryChange();
    _points.append(WirePoint(point - pos()));
    calculateBoundingRect();

    // Update junction
    if (_points.count() > 2) {
        setPointIsJunction(_points.count() - 1, _points.at(_points.count() - 2).isJunction());
        setPointIsJunction(_points.count() - 2, false);
    }

    emit pointInserted(_points.count()-1);
    emit pointMoved(*this, _points.last());
}

void Wire::insertPoint(int index, const QPointF& point)
{
    // Boundary check
    if (index < 0 || index >= _points.count()) {
        return;
    }

    prepareGeometryChange();
    _points.insert(index, WirePoint(_settings.snapToGrid(point - pos())));
    calculateBoundingRect();

    emit pointInserted(index);
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

void Wire::removePoint(const QPointF& point)
{
    prepareGeometryChange();
    _points.removeAll(WirePoint(point - pos()));
    calculateBoundingRect();
}

void Wire::simplify()
{
    prepareGeometryChange();
    removeDuplicatePoints();
    removeObsoletePoints();
    calculateBoundingRect();
}

void Wire::removeDuplicatePoints()
{
    int i = 0;
    while (i < _points.count()-1 and _points.count() > 2) {
        WirePoint p1 = _points.at(i);
        WirePoint p2 = _points.at(i+1);

        // Check if p2 is the same as p1
        if (p1 == p2) {
            // If p1 is not a junction itself then inherit from p2
            if (!p1.isJunction()) {
                setPointIsJunction(i, p2.isJunction());
            }
            emit pointRemoved(i+1);
            _points.removeAt(i+1);
        } else {
            i++;
        }
    }
}

void Wire::removeObsoletePoints()
{
    // Don't do anything if there are not at least three line segments
    if (_points.count() < 3) {
        return;
    }

    // Compile a list of obsolete points
    auto it = _points.begin()+2;
    while (it != _points.end()) {
        QPointF p1 = (*(it - 2)).toPointF();
        QPointF p2 = (*(it - 1)).toPointF();
        QPointF p3 = (*it).toPointF();

        // Check if p2 is on the line created by p1 and p3
        if (Utils::pointIsOnLine(QLineF(p1, p2), p3)) {
            emit pointRemoved(_points.indexOf(*(it-1)));
            it = _points.erase(it-1);
        }
        it++;
    }
}

void Wire::movePointBy(int index, const QVector2D& moveBy)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    // If there are only two points (one line segment) and we are supposed to preserve
    // straight angles, we need to insert two additional points if we are not moving in
    // the direction of the line.
    if (pointsRelative().count() == 2 && _settings.preserveStraightAngles) {
        const Line line = lineSegments().first();

        // Only do this if we're not moving in the direction of the line. Because in that case
        // this is unnecessary as we're just moving one of the two points.
        if ((line.isHorizontal() && !qFuzzyIsNull(moveBy.y())) || (line.isVertical() && !qFuzzyIsNull(moveBy.x()))) {
            qreal lineLength = line.lenght();
            QPointF p;

            // The line is horizontal
            if (line.isHorizontal()) {
                QPointF leftPoint = line.p1();
                if (line.p2().x() < line.p1().x()) {
                    leftPoint = line.p2();
                }

                p.rx() = leftPoint.x() + static_cast<int>(lineLength/2);
                p.ry() = leftPoint.y();

            // The line is vertical
            } else {
                QPointF upperPoint = line.p1();
                if (line.p2().y() < line.p1().y()) {
                    upperPoint = line.p2();
                }

                p.rx() = upperPoint.x();
                p.ry() = upperPoint.y() + static_cast<int>(lineLength/2);
            }

            // Insert twice as these two points will form the new additional vertical or
            // horizontal line segment that is required to preserver straight angles.
            insertPoint(1, p);
            insertPoint(1, p);

            // Account for inserted points
            if (index == 1) {
                index += 2;
            }
        }
    }

    // Move the points
    QPointF currPoint = pointsAbsolute().at(index);
    // Preserve straight angles (if supposed to)
    if (_settings.preserveStraightAngles) {

        // Move previous point
        if (index >= 1) {
            QPointF prevPoint = pointsAbsolute().at(index-1);
            Line line(prevPoint, currPoint);

            // Make sure that two wire points never collide
            if (pointsAbsolute().count() > 3 and index >= 2 and Line(currPoint+moveBy.toPointF(), prevPoint).lenght() <= 2) {
                moveLineSegmentBy(index-2, moveBy);
            }

            // Move junctions before the points are moved
            if (line.isHorizontal() or line.isVertical()) {
                // Move connected junctions
                for (const auto& wire: _connectedWires) {
                    if (wire->isMoving()) {
                        continue;
                    }
                    for (const auto& point: wire->junctions()) {
                        if (line.containsPoint(point.toPointF())) {
                            // Don't move it if it is on one of the points
                            if (line.p1().toPoint() == point.toPoint() or line.p2().toPoint() == point.toPoint()) {
                                continue;
                            }
                            if (line.isHorizontal()) {
                                wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), QVector2D(0, moveBy.y()));
                            } else {
                                wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), QVector2D(moveBy.x(), 0));
                            }
                        }
                    }
                }
            }

            // The line is horizontal
            if (line.isHorizontal()) {
                movePointTo(index-1, pointsAbsolute().at(index-1) + QPointF(0, moveBy.toPointF().y()));
            }

            // The line is vertical
            else if (line.isVertical()) {
                movePointTo(index-1, pointsAbsolute().at(index-1) + QPointF(moveBy.toPointF().x(), 0));
            }
        }

        // Move next point
        if (index < pointsAbsolute().count()-1) {
            QPointF nextPoint = pointsAbsolute().at(index+1);
            Line line(currPoint, nextPoint);

            // Make sure that two wire points never collide
            if (pointsAbsolute().count() > 3 and Line(currPoint+moveBy.toPointF(), nextPoint).lenght() <= 2) {
                moveLineSegmentBy(index+1, moveBy);
            }

            // Move junctions before the points are moved
            if (line.isHorizontal() or line.isVertical()) {
                // Move connected junctions
                for (const auto& wire: _connectedWires) {
                    if (wire->isMoving()) {
                        continue;
                    }
                    for (const auto& point: wire->junctions()) {
                        if (line.containsPoint(point.toPointF())) {
                            // Don't move it if it is on one of the points
                            if (line.p1().toPoint() == point.toPoint() or line.p2().toPoint() == point.toPoint()) {
                                continue;
                            }
                            if (line.isHorizontal()) {
                                wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), QVector2D(0, moveBy.y()));
                            } else {
                                wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), QVector2D(moveBy.x(), 0));
                            }
                        }
                    }
                }
            }

            // The line is horizontal
            if (line.isHorizontal()) {
                movePointTo(index+1, pointsAbsolute().at(index+1) + QPointF(0, moveBy.toPointF().y()));
            }

            // The line is vertical
            else if (line.isVertical()) {
                movePointTo(index+1, pointsAbsolute().at(index+1) + QPointF(moveBy.toPointF().x(), 0));
            }
        }
    }

    // Move the actual point itself
    movePointTo(index, currPoint + moveBy.toPointF());

    simplify();
}

void Wire::movePointTo(int index, const QPointF& moveTo)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    // Do nothing if it already is at that position
    if (pointsAbsolute().at(index) == moveTo) {
        return;
    }

    // Move junctions that are on the point
    for (const auto& wire: _connectedWires) {
        if (wire->isMoving()) {
            continue;
        }
        for (const auto& point: wire->junctions()) {
            if ((_points[index] + pos()).toPoint() == point.toPoint()) {
                wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), QVector2D(moveTo - (_points[index] + pos())));
            }
        }
    }

    // Move junctions on the next segment
    if (index < _points.count()-1) {
        Line segment = lineSegments().at(index);
        Line newSegment(moveTo, pointsAbsolute().at(index+1));
        moveJunctionsToNewSegment(segment, newSegment);
    }

    // Move junctions on the previous segment
    if (index > 0) {
        Line segment = lineSegments().at(index-1);
        Line newSegment(pointsAbsolute().at(index-1), moveTo);
        moveJunctionsToNewSegment(segment, newSegment);
    }

    prepareGeometryChange();
    WirePoint wirepoint = (moveTo - pos());
    wirepoint.setIsJunction(_points[index].isJunction());
    _points[index] = wirepoint;

    emit pointMoved(*this, _points[index]);
    calculateBoundingRect();
    update();
}

void Wire::moveJunctionsToNewSegment(const Line& oldSegment, const Line& newSegment)
{
    // Do nothing if the segment was just resized
    if (qFuzzyCompare(oldSegment.toLineF().angle(), newSegment.toLineF().angle())) {
        return;
    }

    // Move connected junctions
    for (const auto& wire: _connectedWires) {
        if (wire->isMoving()) {
            continue;
        }
        for (const auto& point: wire->junctions()) {
            // Check if the point is on the old segment
            if (oldSegment.containsPoint(point.toPoint(), 5)) {
                int jIndex = wire->wirePointsAbsolute().indexOf(point);
                Line junctionSeg;
                // Find out if one of the segments is horizontal or vertical
                if (jIndex < wire->wirePointsAbsolute().count() - 1) {
                    Line seg = wire->lineSegments().at(jIndex);
                    if (seg.isHorizontal() or seg.isVertical()) {
                        junctionSeg = seg;
                    }
                }
                if (jIndex > 0) {
                    Line seg = wire->lineSegments().at(jIndex-1);
                    if (seg.isHorizontal() or seg.isVertical()) {
                        junctionSeg = seg;
                    }
                }
                // Only move in the direction of the segment if it is hor. or vert.
                if (!junctionSeg.isNull()) {
                    QPointF intersection;
                    auto type = junctionSeg.toLineF().intersect(newSegment.toLineF(), &intersection);
                    if (type != QLineF::NoIntersection) {
                        wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), QVector2D(intersection - point.toPointF()));
                    }
                }
                // Move the point along the segment so that it stays at the same proportional distance from the two points
                else {
                    QPointF d =  point.toPointF() - oldSegment.p1();
                    qreal ratio = QVector2D(d).length() / oldSegment.lenght();
                    QPointF pos = newSegment.toLineF().pointAt(ratio);
                    wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), QVector2D(pos - point.toPointF()));
                }
            }
        }
    }
}

void Wire::moveLineSegmentBy(int index, const QVector2D& moveBy)
{
    // Do nothing if not moving
    if (moveBy.isNull()) {
        return;
    }

    // Have _points.count()-2 in here because N points form N-1 line segments
    if (index < 0 or index > _points.count()-2) {
        return;
    }

    // Move connected junctions
    for (const auto& wire: _connectedWires) {
        if (wire->isMoving()) {
            continue;
        }
        for (const auto& point: wire->junctions()) {
            Line segment = lineSegments().at(index);
            if (segment.containsPoint(point.toPointF())) {
                // Don't move it if it is on one of the points
                if (segment.p1().toPoint() == point.toPoint() or segment.p2().toPoint() == point.toPoint()) {
                    continue;
                }
                wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), moveBy);
            }
        }
    }

    // If this is the first or last segment we might need to add a new segment
    if (index == 0 or index == lineSegments().count() - 1) {
        // Get the correct point
        WirePoint point;
        if (index == 0) {
            point = wirePointsAbsolute().first();
        } else {
            point = wirePointsAbsolute().last();
        }

        bool isConnected = false;
        // Check if the segment is connected to a node
        for (const auto& connector : scene()->connectors()) {
            if (connector->attachedWire() == this and
                connector->attachedWirepoint() == pointsAbsolute().indexOf(point.toPointF())) {
                isConnected = true;
                break;
            }
        }

        // Check if it's connected to a wire
        if (not isConnected and point.isJunction()) {
            isConnected = true;
        }

        // Add segment if it is connected
        if (isConnected) {
            if (index == 0) {
                // Add a point
                prependPoint(_points.first().toPointF() + pos());
                // Increment indices to account for inserted point
                index++;
                _lineSegmentToMoveIndex++;
            } else {
                // Add a point
                appendPoint(_points.last().toPointF() + pos());
            }
        }
    }

    // Move the line segment
    // Move point 1
    {
        const QPointF& newPos = _points[index] + pos() + moveBy.toPointF();
        const std::shared_ptr<Wire>& wirePtr = this->sharedPtr<Wire>();
        auto cmd = new CommandWirepointMove(scene(), wirePtr, index, newPos);
        Q_ASSERT(scene());
        scene()->undoStack()->push(cmd);
    }
    // Move point 2
    {
        const QPointF& newPos = _points[index + 1] + pos() + moveBy.toPointF();
        const std::shared_ptr<Wire>& wirePtr = this->sharedPtr<Wire>();
        auto cmd = new CommandWirepointMove(scene(), wirePtr, index+1, newPos);
        scene()->undoStack()->push(cmd);
    }
}

void Wire::setPointIsJunction(int index, bool isJunction)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    _points[index].setIsJunction(isJunction);

    update();
}

bool Wire::pointIsOnWire(const QPointF& point) const
{
    for (const Line& lineSegment : lineSegments()) {
        if (lineSegment.containsPoint(point, 0)) {
            return true;
        }
    }

    return false;
}

void Wire::connectWire(Wire* wire)
{
    _connectedWires.append(wire);
}

QList<Wire*> Wire::connectedWires()
{
    return _connectedWires;
}

void Wire::disconnectWire(Wire* wire)
{
    _connectedWires.removeAll(wire);
}

QVector<WirePoint> Wire::junctions() const
{
    if (_points.count() < 2) {
        return {};
    }
    QVector<WirePoint> junctions;
    if (_points.first().isJunction()) {
        junctions.append(_points.first() + pos());
    }
    if (_points.last().isJunction()) {
        junctions.append(_points.last() + pos());
    }
    return junctions;
}

QList<Line> Wire::lineSegments() const
{
    // A line segment requires at least two points... duuuh
    if (_points.count() < 2) {
        return QList<Line>();
    }

    QList<Line> ret;
    for (int i = 0; i < _points.count()-1; i++) {
        ret.append(Line(pos() + _points.at(i).toPointF(), pos() + _points.at(i+1).toPointF()));
    }

    return ret;
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Check wheter we clicked on a handle
    if (isSelected()) {
        // Check whether we clicked on a handle
        QVector<QPointF> points(pointsAbsolute());
        _pointToMoveIndex = -1;
        for (int i = 0; i < points.count(); i++) {
            QRectF handleRect(points.at(i).x() - HANDLE_SIZE, points.at(i).y() - HANDLE_SIZE, 2*HANDLE_SIZE, 2*HANDLE_SIZE);

            if (handleRect.contains(event->scenePos())) {
                _pointToMoveIndex = i;
                setMovable(false);
                break;
            }
        }

        // Check whether we clicked on a line segment
        QList<Line> lines = lineSegments();
        for (int i = 0; i < lines.count(); i++) {
            const Line& line = lines.at(i);
            if (line.containsPoint(event->scenePos(), 1)) {
                _lineSegmentToMoveIndex = i;
                setMovable(false);
                break;
            }

            _lineSegmentToMoveIndex = -1;
        }

    } else {
        Item::mousePressEvent(event);
    }

    // Store last known mouse pos
    _prevMousePos = event->scenePos();
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Item::mouseReleaseEvent(event);

    _pointToMoveIndex = -1;
    _lineSegmentToMoveIndex = -1;
    setMovable(true);

    // Store last known mouse pos
    _prevMousePos = event->scenePos();
}

void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF curPos = event->scenePos();
    bool ctrlPressed = QApplication::keyboardModifiers() & Qt::ControlModifier;

    // Snap to grid (if supposed to)
    if (snapToGrid()) {
        curPos = _settings.snapToGrid(curPos);
    }

    // Move a point?
    if (_pointToMoveIndex > -1) {
        // Yep, we can do this
        event->accept();

        // Move
        auto wire = this->sharedPtr<Wire>();
        auto command = new CommandWirepointMove(scene(), wire, _pointToMoveIndex, curPos);
        scene()->undoStack()->push(command);
    }

    // Move a line segment?
    else if (_lineSegmentToMoveIndex > -1){
        // Yep, we can do this
        event->accept();

        // Determine movement vector
        const Line line = lineSegments().at(_lineSegmentToMoveIndex);
        QVector2D moveLineBy(0, 0);
        if (line.isHorizontal()) {
            moveLineBy = QVector2D(0, static_cast<float>(curPos.y() - _prevMousePos.y()));
        } else if (line.isVertical()) {
            moveLineBy = QVector2D(static_cast<float>(curPos.x() - _prevMousePos.x()), 0);
        } else if (ctrlPressed) {
            moveLineBy = QVector2D(curPos - _prevMousePos);
        }

        // Snap to grid (if supposed to)
        if (snapToGrid()) {
            moveLineBy = _settings.snapToGrid(moveLineBy);
        }

        // Move line segment
        moveLineSegmentBy(_lineSegmentToMoveIndex, moveLineBy);
    }

    // Nothing interesting for us to do
    else {

        // Hand over to base class
        Item::mouseMoveEvent(event);
    }

    // Store last known mouse pos
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
    QVector<QPointF> points(pointsAbsolute());
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
        // Retrieve the line segment
        const Line& line = lines.at(i);

        // Set the appropriate cursor
        if (line.containsPoint(event->scenePos(), 1)) {
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
    const auto& points = pointsRelative();
    painter->drawPolyline(points.constData(), points.count());

    // Draw the junction poins
    int junctionRadius = 4;
    for (const WirePoint& wirePoint : wirePointsRelative()) {
        if (wirePoint.isJunction()) {
            painter->setPen(penJunction);
            painter->setBrush(brushJunction);
            painter->drawEllipse(wirePoint.toPointF(), junctionRadius, junctionRadius);
        }
    }

    // Draw the handles (if selected)
    if (isSelected()) {
        painter->setOpacity(0.5);
        painter->setPen(penHandle);
        painter->setBrush(brushHandle);
        for (const QPointF& point : points) {
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

QVariant Wire::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change) {

    case ItemPositionChange: {
        // Move the wire
        QPointF newPos = QPointF(_settings.snapToGrid(value.toPointF())) + _offset;
        QVector2D movedBy = QVector2D(newPos - pos());
        // Move junctions
        if (not _internalMove and scene()) {
            for (const auto& junction : junctions()) {
                for (const auto& wire : scene()->wires()) {
                    if (not wire->connectedWires().contains(this)) {
                        continue;
                    }
                    if (wire->isMoving()) {
                        continue;
                    }
                    if (wire->pointIsOnWire(junction.toPointF()) and not movedBy.isNull()) {
                        int index = wirePointsAbsolute().indexOf(junction);
                        movePointBy(index, -movedBy);
                    }
                }
            }
            // Move junction on the wire
            for (const auto& wire : connectedWires()) {
                if (wire->isMoving()) {
                    continue;
                }
                for (const auto& point : wire->junctions()) {
                    if (pointIsOnWire(point.toPointF())) {
                        wire->movePointBy(wire->wirePointsAbsolute().indexOf(point), movedBy);
                    }
                }
            }
        }
        return newPos;
    }
    case ItemPositionHasChanged:
        if (_internalMove) {
            break;
        }
        if (not scene()) {
            break;
        }
        // Move points to their connectors
        for (const auto& conn : scene()->connectors()) {
            bool isSelected = false;
            // Check if the connector's node is selected
            for (const auto& item : scene()->selectedTopLevelItems()) {
                auto node = item->sharedPtr<Node>();
                if (node) {
                    if (node->connectors().contains(conn)) {
                        isSelected = true;
                        break;
                    }
                }
            }
            // Move point onto the connector
            if (not isSelected and conn->attachedWire() == this) {
                int index = conn->attachedWirepoint();
                QVector2D moveBy(conn->scenePos() - pointsAbsolute().at(index));
                movePointBy(index, moveBy);
            }
        }
        break;
    case ItemSelectedHasChanged:
        if (value.toBool()) {
            setZValue(zValue()+1);
        } else {
            setZValue(zValue()-1);
        }
        break;
    default:
        return Item::itemChange(change, value);
    }
    return Item::itemChange(change, value);
}

void Wire::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    if (_renameAction) {
        menu.addAction(_renameAction);
    }
    if (not net()->label()->text().isEmpty()) {
        QAction* showAction = menu.addAction("Label visible");
        showAction->setCheckable(true);
        showAction->setChecked(net()->label()->isVisible());

        connect(showAction, &QAction::triggered, this, &Wire::toggleLabelRequested);
    }
    bool labelWasVisible = net()->label()->isVisible();
    if (menu.actions().count()) {
        menu.exec(event->screenPos());
    }
    // Move the label to the cursor if it was just made visible
    if (not labelWasVisible and net()->label()->isVisible()) {
        // Find line segment
        Line seg;
        QList<Line> lines = lineSegments();
        for (const auto& line : lines) {
            if (line.containsPoint(event->scenePos(), WIRE_SHAPE_PADDING/2)) {
                seg = line;
                break;
            }

        }
        // This should never happen
        if (seg.isNull()) {
            qCritical("Wire::contextMenuEvent(): Couldn't identify the segment the user clicked on.");
            return;
        }
        // Offset the position
        QPointF pos = event->scenePos();
        qreal angle = QLineF(seg.p1(), seg.p2()).angle();
        // When the wire is horizontal move the label up
        if (seg.isHorizontal()) {
            pos.setY(seg.p1().y() - _settings.gridSize / 2);
        }
        // When the wire is vertical move the label to the right
        else if (seg.isVertical()) {
            pos.setX(seg.p1().x() + _settings.gridSize / 2);
        }
        // When the wire is diagonal with a positive slope move it up and to the left
        else if ((angle > 0 and angle < 90) or (angle > 180 and angle < 360)) {
            QPointF point = Utils::pointOnLineClosestToPoint(seg.p1(), seg.p2(), pos);
            pos.setX(point.x() - _settings.gridSize / 2 - net()->label()->textRect().width());
            pos.setY(point.y() - _settings.gridSize / 2);
        }
        // When the wire is diagonal with a negative slope move it up and to the right
        else {
            QPointF point = Utils::pointOnLineClosestToPoint(seg.p1(), seg.p2(), pos);
            pos.setX(point.x() + _settings.gridSize / 2);
            pos.setY(point.y() - _settings.gridSize / 2);
        }
        net()->label()->setParentItem(this);
        net()->label()->setPos(pos - Wire::pos());
    }
}

std::shared_ptr<WireNet> Wire::net()
{
    return _net;
}

void Wire::setNet(const std::shared_ptr<WireNet>& wirenet)
{
    _net = wirenet;
}

bool Wire::movingWirePoint() const
{
    if (_pointToMoveIndex >= 0 or _lineSegmentToMoveIndex >= 0) {
        return true;
    } else {
        return false;
    }
}
