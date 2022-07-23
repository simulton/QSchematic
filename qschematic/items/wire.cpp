#include "wire.h"
#include "connector.h"
#include "label.h"
#include "node.h"
#include "../scene.h"
#include "../utils.h"
#include "../commands/commandwirepointmove.h"

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
    Item(type, parent), _renameAction(nullptr)
{
    _pointToMoveIndex = -1;
    _lineSegmentToMoveIndex = -1;

    // Lines should always be the lowest item in Z-Order
    setZValue(-10);

    // ALWAYS snap to grid
    setSnapToGrid(true);
    setMovable(true);
}

Wire::~Wire()
{
    if (auto wire_net = std::dynamic_pointer_cast<WireNet>(net())) {
        // Make sure that we don't delete the net's label
        if (childItems().contains(wire_net->label().get())) {
            wire_net->label()->setParentItem(nullptr);
        }
    }
}

gpds::container Wire::to_container() const
{
    // Points
    gpds::container pointsContainer;
    for (int i = 0; i < points_count(); i++) {
        gpds::container pointContainer;
        pointContainer.add_attribute("index", i);
        pointContainer.add_value("x", m_points.at(i).x());
        pointContainer.add_value("y", m_points.at(i).y());
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
            m_points.append(point(pointContainer->get_value<double>("x").value_or(0),
                                  pointContainer->get_value<double>("y").value_or(0)));
        }
    }

    update();
}

std::shared_ptr<Item> Wire::deepCopy() const
{
    auto clone = std::make_shared<Wire>(type(), parentItem());
    copyAttributes(*clone);

    return clone;
}

void Wire::copyAttributes(Wire& dest) const
{
    Item::copyAttributes(dest);

    dest.m_points = m_points;
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

QVector<point> Wire::wirePointsRelative() const
{
    QVector<point> relativePoints(m_points);

    for (point& point : relativePoints) {
        bool isJunction = point.is_junction();
        point = point.toPointF() - pos();
        point.set_is_junction(isJunction);
    }

    return relativePoints;
}

QVector<QPointF> Wire::pointsRelative() const
{
    QVector<QPointF> points;

    for (const point& point : m_points) {
        points << point.toPointF() - pos();
    }

    return points;
}

QVector<QPointF> Wire::pointsAbsolute() const
{
    QVector<QPointF> points;

    for (const point& point : m_points) {
        points << point.toPointF();
    }

    return points;
}

void Wire::calculateBoundingRect()
{
    // Find top-left most point
    const int& intMaxValue = std::numeric_limits<int>::max();
    QPointF topLeft(intMaxValue, intMaxValue);
    for (auto& point : wirePointsRelative()) {
        if (point.x() < topLeft.x())
            topLeft.setX(point.x());
        if (point.y() < topLeft.y())
            topLeft.setY(point.y());
    }

    // Find bottom-right most point
    const int& intMinValue = std::numeric_limits<int>::min();
    QPointF bottomRight(intMinValue, intMinValue);
    for (auto& point : wirePointsRelative()) {
        if (point.x() > bottomRight.x())
            bottomRight.setX(point.x());
        if (point.y() > bottomRight.y())
            bottomRight.setY(point.y());
    }

    // Create the rectangle
    _rect = QRectF(topLeft, bottomRight);
}

void Wire::setRenameAction(QAction* action)
{
    _renameAction = action;
}

void Wire::prepend_point(const QPointF& point)
{
    wire::prepend_point(point);
    emit pointMoved(*this, wirePointsRelative().first());
}

void Wire::append_point(const QPointF& point)
{
    wire::append_point(point);
    emit pointMoved(*this, wirePointsRelative().last());
}

void Wire::insert_point(int index, const QPointF& point)
{
    wire::insert_point(index, point);
    emit pointMoved(*this, wirePointsRelative()[index]);
}

void Wire::removeFirstPoint()
{
    if (points_count() <= 0) {
        return;
    }
    prepareGeometryChange();
    m_points.removeFirst();
    calculateBoundingRect();
}

void Wire::removeLastPoint()
{
    if (points_count() <= 0) {
        return;
    }

    prepareGeometryChange();
    m_points.removeLast();
    calculateBoundingRect();
}

void Wire::move_point_to(int index, const QPointF& moveTo)
{
    prepareGeometryChange();
    wire_system::wire::move_point_to(index, moveTo);

    emit pointMoved(*this, wirePointsRelative()[index]);
    calculateBoundingRect();
    update();
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
        QList<line> lines = line_segments();
        for (int i = 0; i < lines.count(); i++) {
            const line& line = lines.at(i);
            if (line.contains_point(event->scenePos(), 1)) {
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
    simplify();
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
        const line line = line_segments().at(_lineSegmentToMoveIndex);
        QVector2D moveLineBy(0, 0);
        if (line.is_horizontal()) {
            moveLineBy = QVector2D(0, static_cast<float>(curPos.y() - _prevMousePos.y()));
        } else if (line.is_vertical()) {
            moveLineBy = QVector2D(static_cast<float>(curPos.x() - _prevMousePos.x()), 0);
        } else if (ctrlPressed) {
            moveLineBy = QVector2D(curPos - _prevMousePos);
        }

        // Snap to grid (if supposed to)
        if (snapToGrid()) {
            moveLineBy = _settings.snapToGrid(moveLineBy);
        }

        // Move line segment
        move_line_segment_by(_lineSegmentToMoveIndex, moveLineBy);
    }

    // Nothing interesting for us to do
    else {

        // Hand over to base class
        Item::mouseMoveEvent(event);
    }

    // Store last known mouse pos
    _prevMousePos = curPos;
}

void Wire::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    // Ignore if there is no rename action
    if (!_renameAction) {
        return;
    }

    // If the net is a WireNet, retrieve the label
    std::shared_ptr<Label> label;
    if (auto wireNet = std::dynamic_pointer_cast<WireNet>(net())) {
        label = wireNet->label();
    }

    // Ignore if there is no label
    if (!label) {
        return;
    }

    // Keep track of whether the label is already visible
    bool labelWasVisible = label->isVisible();

    // Trigger the action
    _renameAction->trigger();

    // Move the label to the cursor if it wasn't already visible
    if (!labelWasVisible && label->isVisible()) {
        label_to_cursor(event->scenePos(), label);
    }
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
    QList<line> lines = line_segments();
    for (int i = 0; i < lines.count(); i++) {
        // Retrieve the line segment
        const line& line = lines.at(i);

        // Set the appropriate cursor
        if (line.contains_point(event->scenePos(), 1)) {
            if (line.is_horizontal()) {
                setCursor(Qt::SizeVerCursor);
            } else if (line.is_vertical()) {
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
    for (const point& wirePoint : wirePointsRelative()) {
        if (wirePoint.is_junction()) {
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
        move(movedBy);
        return newPos;
    }
    case ItemPositionHasChanged:
        if (!scene()) {
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
            if (!isSelected && scene()->wire_manager()->attached_wire(conn.get()) == this) {
                int index = scene()->wire_manager()->attached_point(conn.get());
                QVector2D moveBy(conn->scenePos() - pointsAbsolute().at(index));
                move_point_by(index, moveBy);
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
    QAction* actionAdd = menu.addAction("Add point");
    // If there is a point nearby
    int pointIndex = -1;
    for (int i = 0; i < points_count(); i++) {
        if (QVector2D(pointsAbsolute().at(i)).distanceToPoint(QVector2D(event->scenePos())) < 5) {
            pointIndex = i;
            break;
        }
    }
    QAction* actionRemove = nullptr;
    if (points_count() > 2 && pointIndex != -1) {
        actionRemove = menu.addAction("Remove point");
    }
    if (_renameAction) {
        menu.addAction(_renameAction);
    }

    // If the net is a WireNet, retrieve the label
    std::shared_ptr<Label> label;
    if (auto wireNet = std::dynamic_pointer_cast<WireNet>(net())) {
        label = wireNet->label();
    }

    // Show checkbox to toggle the visibility of the label
    if (label && !net()->name().isEmpty()) {
        QAction* showAction = menu.addAction("Label visible");
        showAction->setCheckable(true);
        showAction->setChecked(label->isVisible());

        connect(showAction, &QAction::triggered, this, &Wire::toggleLabelRequested);
    }
    bool labelWasVisible = label && label->isVisible();
    QAction* command = menu.exec(event->screenPos());

    // Add a point at the cursor
    if (command == actionAdd) {
        for (int i = 0; i < line_segments().count(); i++) {
            if (line_segments().at(i).contains_point(event->scenePos(), 4)) {
                setSelected(true);
                insert_point(i + 1, _settings.snapToGrid(event->scenePos()));
                break;
            }
        }
    }

    // Remove the point near the cursor
    if (actionRemove && command == actionRemove) {
        remove_point(pointIndex);
    }

    // Move the label to the cursor if it was just made visible
    if (label && !labelWasVisible && label->isVisible()) {
        label_to_cursor(event->scenePos(), label);
    }
}

void Wire::label_to_cursor(const QPointF& scenePos, std::shared_ptr<Label>& label) const
{
    // Find line segment
    line seg;
    QList<line> lines = line_segments();
    for (const auto& line : lines) {
        if (line.contains_point(scenePos, WIRE_SHAPE_PADDING / 2)) {
            seg = line;
            break;
        }

    }
    // This should never happen
    if (seg.is_null()) {
        qCritical("Wire::contextMenuEvent(): Couldn't identify the segment the user clicked on.");
        return;
    }
    // Offset the position
    QPointF pos = scenePos;
    qreal angle = QLineF(seg.p1(), seg.p2()).angle();
    // When the wire is horizontal move the label up
    if (seg.is_horizontal()) {
        pos.setY(seg.p1().y() - _settings.gridSize / 2);
    }
    // When the wire is vertical move the label to the right
    else if (seg.is_vertical()) {
        pos.setX(seg.p1().x() + _settings.gridSize / 2);
    }
    // When the wire is diagonal with a positive slope move it up and to the left
    else if ((angle > 0 && angle < 90) || (angle > 180 && angle < 360)) {
        QPointF point = Utils::pointOnLineClosestToPoint(seg.p1(), seg.p2(), pos);
        pos.setX(point.x() - _settings.gridSize / 2 - label->textRect().width());
        pos.setY(point.y() - _settings.gridSize / 2);
    }
    // When the wire is diagonal with a negative slope move it up and to the right
    else {
        QPointF point = Utils::pointOnLineClosestToPoint(seg.p1(), seg.p2(), pos);
        pos.setX(point.x() + _settings.gridSize / 2);
        pos.setY(point.y() - _settings.gridSize / 2);
    }
    label->setParentItem((QGraphicsItem*) this);
    label->setPos(pos - Wire::pos());
}

bool Wire::movingWirePoint() const
{
    if (_pointToMoveIndex >= 0 || _lineSegmentToMoveIndex >= 0) {
        return true;
    } else {
        return false;
    }
}

void Wire::about_to_change()
{
    prepareGeometryChange();
}

void Wire::has_changed()
{
    calculateBoundingRect();
}

void Wire::add_segment(int index)
{
    if (index == 0) {
        _lineSegmentToMoveIndex++;
    }
    wire::add_segment(index);
}

void Wire::rename_net()
{
    if (_renameAction) {
        _renameAction->trigger();
    }
}
