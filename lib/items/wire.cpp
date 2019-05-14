#include <QPen>
#include <QBrush>
#include <QPainter>
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
    setMovable(false);
}

Gpds::Container Wire::toContainer() const
{
    // Points
    Gpds::Container pointsContainer;
    for (int i = 0; i < _points.count(); i++) {
        Gpds::Container pointContainer;
        pointContainer.addAttribute("index", QString::number(i));
        pointContainer.addValue("x", _points.at(i).x());
        pointContainer.addValue("y", _points.at(i).y());
        pointsContainer.addValue("point", pointContainer);
    }

    // Root
    Gpds::Container rootContainer;
    addItemTypeIdToContainer(rootContainer);
    rootContainer.addValue("item", Item::toContainer());
    rootContainer.addValue("points", pointsContainer);

    return rootContainer;
}

void Wire::fromContainer(const Gpds::Container& container)
{
    // Root
    Item::fromContainer( *container.getValue<Gpds::Container*>( "item" ) );

    // Points
    const Gpds::Container* pointsContainer = container.getValue<Gpds::Container*>( "points" );
    if (pointsContainer) {
        for (const Gpds::Container* pointContainer : pointsContainer->getValues<Gpds::Container*>( "point" ) ) {
#warning ToDo: Get index argument
            _points.append( WirePoint( pointContainer->getValue<double>("x"), pointContainer->getValue<double>("y") ) );
        }
    }

    update();
}

std::unique_ptr<Item> Wire::deepCopy() const
{
    auto clone = std::make_unique<Wire>(type(), parentItem());
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
        point = point + pos();
    }

    return absolutePoints;
}

QVector<QPointF> Wire::pointsRelative() const
{
    QVector<QPointF> points;

    for (const WirePoint& point : _points) {
        points << point.toPoint();
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
}

void Wire::prependPoint(const QPointF& point)
{
    prepareGeometryChange();
    _points.prepend(WirePoint(point - gridPos()));
    calculateBoundingRect();

    emit pointMoved(*this, _points.first());
}

void Wire::appendPoint(const QPointF& point)
{
    prepareGeometryChange();
    _points.append(WirePoint(point - gridPos()));
    calculateBoundingRect();

    emit pointMoved(*this, _points.last());
}

void Wire::insertPoint(int index, const QPointF& point)
{
    // Boundary check
    if (index < 0 || index >= _points.count()) {
        return;
    }

    prepareGeometryChange();
    _points.insert(index, WirePoint(point - gridPos()));
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

void Wire::removePoint(const QPointF& point)
{
    prepareGeometryChange();
    _points.removeAll(WirePoint(point - gridPos()));
    calculateBoundingRect();
}

void Wire::simplify()
{
    prepareGeometryChange();
    simplify(_points);
    calculateBoundingRect();
}

void Wire::simplify(QVector<WirePoint>& points)
{
    removeDuplicatePoints(points);
    removeObsoletePoints(points);
}

void Wire::removeDuplicatePoints(QVector<WirePoint>& points)
{
    QVector<WirePoint> newList;

    for (auto it = points.begin(); it != points.end(); it++) {
        if (!newList.contains(*it)) {
            newList << *it;
        }
    }

    points = std::move(newList);
}

void Wire::removeObsoletePoints(QVector<WirePoint>& points)
{
    // Don't do anything if there are not at least three line segments
    if (points.count() < 3) {
        return;
    }

    // Compile a list of obsolete points
    auto it = points.begin()+2;
    while (it != points.end()) {
        QPoint p1 = (*(it - 2)).toPoint();
        QPoint p2 = (*(it - 1)).toPoint();
        QPoint p3 = (*it).toPoint();

        // Check if p2 is on the line created by p1 and p3
        if (Utils::pointIsOnLine(QLineF(p1, p2), p3)) {
            it = points.erase(it-1);
        } else {
            it++;
        }
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

void Wire::movePointTo(int index, const QPointF& moveTo)
{
    if (index < 0 or index > _points.count()-1) {
        return;
    }

    prepareGeometryChange();
    _points[index] = WirePoint(moveTo - gridPos());
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

bool Wire::pointIsOnWire(const QPointF& point) const
{
    for (const Line& lineSegment : lineSegments()) {
        if (lineSegment.containsPoint(point, 0)) {
            return true;
        }
    }

    return false;
}

QList<Line> Wire::lineSegments() const
{
    // A line segment requires at least two points... duuuh
    if (_points.count() < 2) {
        return QList<Line>();
    }

    QList<Line> ret;
    for (int i = 0; i < _points.count()-1; i++) {
        ret.append(Line(gridPos() + _points.at(i).toPoint(), gridPos() + _points.at(i+1).toPoint()));
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
                break;
            }
        }

        // Check whether we clicked on a line segment
        QList<Line> lines = lineSegments();
        for (int i = 0; i < lines.count(); i++) {
            const Line& line = lines.at(i);
            if (line.containsPoint(event->scenePos(), 1)) {
                _lineSegmentToMoveIndex = i;
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

    // Store last known mouse pos
    _prevMousePos = event->scenePos();
}
#include <QtDebug>
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
        movePointTo(_pointToMoveIndex, curPos);

    }

    // Move a line segment?
    else if (_lineSegmentToMoveIndex > -1){
        // Yep, we can do this
        event->accept();

        // Determine movement vector
        const Line& line = lineSegments().at(_lineSegmentToMoveIndex);
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
    _prevMousePos = event->scenePos();
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
            painter->drawEllipse(wirePoint.toPoint(), junctionRadius, junctionRadius);
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
