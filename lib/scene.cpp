#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QUndoStack>
#include <QPixmap>
#include <QMimeData>
#include "scene.h"
#include "settings.h"
#include "commands/commanditemmove.h"
#include "commands/commanditemadd.h"
#include "items/itemfactory.h"
#include "items/item.h"
#include "items/itemmimedata.h"
#include "items/node.h"
#include "items/connector.h"
#include "items/wire.h"
#include "items/wirenet.h"

using namespace QSchematic;

Scene::Scene(QObject* parent) :
    QGraphicsScene(parent),
    _mode(NormalMode),
    _newWireSegment(false),
    _invertWirePosture(true)
{
    // Undo stack
    _undoStack = new QUndoStack;
    connect(_undoStack, &QUndoStack::cleanChanged, [this](bool isClean) {
        emit isDirtyChanged(!isClean);
    });

    // Stuff
    connect(this, &QGraphicsScene::sceneRectChanged, [this]{
        renderCachedBackground();
    });

    // Prepare the background
    renderCachedBackground();
}

QJsonObject Scene::toJson() const
{
    QJsonObject object;

    // Scene rect
    {
        QJsonObject rectObject;
        const QRect& rect = sceneRect().toRect();
        rectObject.insert("x", rect.x());
        rectObject.insert("y", rect.y());
        rectObject.insert("width", rect.width());
        rectObject.insert("height", rect.height());

        object.insert("scene rect", rectObject);
    }

    // Nodes
    QJsonArray itemsArray;
    for (const auto& node : nodes()) {
        itemsArray.append(node->toJson());
    }
    object.insert("nodes", itemsArray);

    // WireNets
    QJsonArray netsArray;
    for (const auto& net : nets()) {
        netsArray.append(net->toJson());
    }
    object.insert("nets", netsArray);

    return object;
}

bool Scene::fromJson(const QJsonObject& object)
{
    Q_UNUSED(object)

    // Scene rect
    {
        const QJsonObject& rectObject = object["scene rect"].toObject();

        QRect sceneRect;
        sceneRect.setX(rectObject["x"].toInt());
        sceneRect.setY(rectObject["y"].toInt());
        sceneRect.setWidth(rectObject["width"].toInt());
        sceneRect.setHeight(rectObject["height"].toInt());
        setSceneRect(sceneRect);
    }

    // Nodes
    {
        QJsonArray array = object["nodes"].toArray();
        for (const QJsonValue& value : array) {
            QJsonObject object = value.toObject();
            if (!object.isEmpty()) {
                std::unique_ptr<Item> node = ItemFactory::instance().fromJson(object);
                if (!node) {
                    qCritical("Scene::fromJson(): Couldn't restore node. Skipping.");
                    continue;
                }
                node->fromJson(object);
                addItem(std::move(node));
            }
        }
    }

    // WireNets
    {
        QJsonArray array = object["nets"].toArray();
        for (const QJsonValue& value : array) {
            QJsonObject object = value.toObject();
            if (!object.isEmpty()) {
                auto net = std::make_unique<WireNet>();
                net->fromJson(object);

                for (auto& wire : net->wires()) {
                    addItem(wire);
                }

                addWireNet(std::move(net));
            }
        }
    }

    // Clear the undo history
    _undoStack->clear();

    return true;
}

void Scene::setSettings(const Settings& settings)
{
    // Update settings of all items
    for (auto& item : items()) {
        item->setSettings(settings);
    }

    // Store new settings
    _settings = settings;

    // Redraw
    renderCachedBackground();
    update();
}

void Scene::setWireFactory(const std::function<std::unique_ptr<Wire>()>& factory)
{
    _wireFactory = factory;
}

void Scene::setMode(Scene::Mode mode)
{
    // Dont do anything unnecessary
    if (mode == _mode) {
        return;
    }

    // Check what the previous mode was
    switch (_mode) {

    // Discard current wire/bus
    case WireMode:
        _newWire.reset();
        break;

    default:
        break;

    }

    // Store the new mode
    _mode = mode;

    // Update the UI
    update();

    // Let the world know
    emit modeChanged(_mode);
}
void Scene::toggleWirePosture()
{
    _invertWirePosture = !_invertWirePosture;
}

bool Scene::isDirty() const
{
    Q_ASSERT(_undoStack);

    return !_undoStack->isClean();
}

void Scene::clearIsDirty()
{
    Q_ASSERT(_undoStack);

    _undoStack->setClean();
}

void Scene::clear()
{
    // Remove from scene
    // Do not use QGraphicsScene::clear() as that would also delete the items. However,
    // we still need them as we manage them via smart pointers (eg. in commands)
    for (auto& item : _items) {
        removeItem(item);
    }
    Q_ASSERT(_items.isEmpty());

    // Nets
    _nets.clear();
    Q_ASSERT(_nets.isEmpty());

    // Selected items
    _selectedItems.clear();
    Q_ASSERT(_selectedItems.isEmpty());

    // Undo stack
    _undoStack->clear();
    clearIsDirty();

    // Update
    update();
}

bool Scene::addItem(const std::shared_ptr<Item>& item)
{
    // Sanity check
    if (!item) {
        return false;
    }

    // Setup item
    setupNewItem(*(item.get()));

    // Add to scene
    QGraphicsScene::addItem(item.get());

    // Store the shared pointer to keep the item alive for the QGraphicsScene
    _items << item;

    // Let the world know
    emit itemAdded(item);

    return true;
}

bool Scene::removeItem(const std::shared_ptr<Item>& item)
{
    // Sanity check
    if (!item) {
        return false;
    }

    // Remove from scene
    QGraphicsScene::removeItem(item.get());

    // Remove shared pointer from local list to reduce instance count
    _items.removeAll(item);

    // Let the world know
    emit itemRemoved(item);

    return true;
}

QList<std::shared_ptr<Item>> Scene::items() const
{
    return _items;
}

QList<std::shared_ptr<Item>> Scene::items(int itemType) const
{
    QList<std::shared_ptr<Item>> items;

    for (auto& item : _items) {
        if (item->type() != itemType) {
            continue;
        }

        items << item;
    }

    return items;
}

QVector<std::shared_ptr<Item>> Scene::selectedItems() const
{
    // Retrieve items from QGraphicsScene
    const auto& rawItems = QGraphicsScene::selectedItems();

    // Retrieve corresponding smart pointers
    QVector<std::shared_ptr<Item>> items(rawItems.count());
    int i = 0;
    for (auto& item : _items) {
        if (rawItems.contains(item.get())) {
            items[i++] = item;
        }
    }

    return items;
}

QList<std::shared_ptr<Node>> Scene::nodes() const
{
    QList<std::shared_ptr<Node>> nodes;

    for (auto& item : _items) {
        auto node = std::dynamic_pointer_cast<Node>(item);
        if (!node) {
            continue;
        }

        nodes << node;
    }

    return nodes;
}

bool Scene::addWire(const std::shared_ptr<Wire>& wire)
{
    // Sanity check
    if (!wire) {
        return false;
    }

    // Check if any point of the wire lies on any line segment of all existing line segments.
    // If yes, add to that net. Otherwise, create a new one
    for (auto& net : _nets) {
        for (const Line& line : net->lineSegments()) {
            for (const WirePoint& point : wire->pointsRelative()) {
                if (line.containsPoint(point.toPoint(), 0)) {
                    net->addWire(wire);
                    return true;
                }
            }
        }
    }

    // Check if any line segment of the wire lies on any point of all existing wires.
    // If yes, add to that net. Otherwise, create a new one
    for (auto& net : _nets) {
        for (const auto& otherWire : net->wires()) {
            for (const WirePoint& otherPoint : otherWire->wirePointsRelative()) {
                for (const Line& line : wire->lineSegments()) {
                    if (line.containsPoint(otherPoint.toPoint())) {
                        net->addWire(wire);
                        return true;
                    }
                }
            }
        }
    }

    // No point of the new wire lies on an existing line segment - create a new wire net
    auto newNet = std::make_unique<WireNet>();
    newNet->addWire(wire);
    addWireNet(std::move(newNet));

    // Add wire to scene
    // Wires createde by mouse interactions are already added to the scene in the Scene::mouseXxxEvent() calls. Prevent
    // adding an already added item to the scene
    if (wire->QGraphicsItem::scene() != this) {
        if (!addItem(wire)) {
            return false;
        }
    }

    return true;
}

bool Scene::removeWire(const std::shared_ptr<Wire>& wire)
{
    // Remove the wire from the scene
    removeItem(wire);

    // Remove the wire from the list
    QList<std::shared_ptr<WireNet>> netsToDelete;
    for (auto& net : _nets) {
        if (net->contains(wire)) {
            net->removeWire(wire);
        }

        if (net->wires().count() < 1) {
            netsToDelete.append(net);
        }
    }

    // Delete the net if this was the nets last wire
    for (auto& net : netsToDelete) {
        _nets.removeAll(net);
    }

    return true;
}

QList<std::shared_ptr<Wire>> Scene::wires() const
{
    QList<std::shared_ptr<Wire>> list;

    for (const auto& wireNet : _nets) {
        list.append(wireNet->wires());
    }

    return list;
}

QList<std::shared_ptr<WireNet>> Scene::nets() const
{
    return _nets;
}

QList<std::shared_ptr<WireNet>> Scene::nets(const std::shared_ptr<WireNet>& wireNet) const
{
    QList<std::shared_ptr<WireNet>> list;

    for (auto& net : _nets) {
        if (!net) {
            continue;
        }

        if (net->name().isEmpty()) {
            continue;
        }

        if (QString::compare(net->name(), wireNet->name(), Qt::CaseInsensitive) == 0) {
            list.append(net);
        }
    }

    return list;
}

std::shared_ptr<WireNet> Scene::net(const std::shared_ptr<Wire>& wire) const
{
    for (auto& net : _nets) {
        for (const auto& w : net->wires()) {
            if (w == wire) {
                return net;
            }
        }
    }

    return nullptr;
}

QList<std::shared_ptr<WireNet>> Scene::netsAt(const QPoint& point)
{
    QList<std::shared_ptr<WireNet>> list;

    for (auto& net : _nets) {
        for (const Line& line : net->lineSegments()) {
            if (line.containsPoint(point) && !list.contains(net)) {
                list.append(net);
            }
        }
    }

    return list;
}

void Scene::undo()
{
    _undoStack->undo();
}

void Scene::redo()
{
    _undoStack->redo();
}

QUndoStack* Scene::undoStack() const
{
    return _undoStack;
}

void Scene::itemMoved(const Item& item, const QVector2D& movedBy)
{
    // Nothing to do if the item didn't move at all
    if (movedBy.isNull()) {
        return;
    }

    // If this is a Node class, move wires with it
    const Node* node = dynamic_cast<const Node*>(&item);
    if (node) {
        // Create a list of all wires there were connected to the SchematicObject
        auto wiresConnectedToMovingObjects = wiresConnectedTo(*node, movedBy*(-1));

        // Update wire positions
        for (auto& wire : wiresConnectedToMovingObjects) {
            for (const QPointF& connectionPoint : node->connectionPoints()) {
                wireMovePoint(connectionPoint, *wire, movedBy);
            }
        }

        // Clean up the wires
        for (const auto& wire : wiresConnectedToMovingObjects) {
            auto wireNet = net(wire);
            if (!wireNet) {
                continue;
            }

            wireNet->simplify();
        }
    }
}

void Scene::wireNetHighlightChanged(bool highlighted)
{
    auto rawPointer = qobject_cast<WireNet*>(sender());
    if (!rawPointer) {
        return;
    }
    std::shared_ptr<WireNet> wireNet;
    for (auto& wn : _nets) {
        if (wn.get() == rawPointer) {
            wireNet = wn;
            break;
        }
    }
    if (!wireNet) {
        return;
    }

    // Highlight all wire nets that are part of this net
    for (auto& otherWireNet : nets(wireNet)) {
        if (otherWireNet == wireNet) {
            continue;
        }

        otherWireNet->blockSignals(true);
        otherWireNet->setHighlighted(highlighted);
        otherWireNet->blockSignals(false);
    }
}

void Scene::wirePointMoved(Wire& rawWire, WirePoint& point)
{
    Q_UNUSED(point)

    // Retrieve corresponding shared ptr
    std::shared_ptr<Wire> wire;
    for (auto& item : _items) {
        std::shared_ptr<Wire> wireItem = std::dynamic_pointer_cast<Wire>(item);
        if (!wireItem) {
            continue;
        }

        if (wireItem.get() == &rawWire) {
            wire = wireItem;
            break;
        }
    }

    // Remove the Wire from the current WireNet if it is part of a WireNet
    auto it = _nets.begin();
    while (it != _nets.end()) {
        // Alias the Net
        auto& net = *it;

        // Remove the Wire from the Net
        if (net->contains(wire)) {
            net->removeWire(wire);
            net->setHighlighted(false);

            // Remove the WireNet if it has no more Wires
            if (net->wires().isEmpty()) {
                it = _nets.erase(it);
            }

            // A Wire can only be part of one WireNet - therefore, we're done
            break;

        } else {

            it++;

        }
    }

    // Add the wire
    addWire(wire);
}

void Scene::wireMovePoint(const QPointF& point, Wire& wire, const QVector2D& movedBy) const
{
    // If there are only two points (one line segment) and we are supposed to preserve
    // straight angles, we need to insert two additional points if we are not moving in
    // the direction of the line.
    if (wire.pointsRelative().count() == 2 && _settings.preserveStraightAngles) {
        const Line& line = wire.lineSegments().first();

        // Only do this if we're not moving in the direction of the line. Because in that case
        // this is unnecessary as we're just moving one of the two points.
        if ((line.isHorizontal() && !qFuzzyIsNull(movedBy.y())) || (line.isVertical() && !qFuzzyIsNull(movedBy.x()))) {
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
                if (line.p2().x() < line.p1().x()) {
                    upperPoint = line.p2();
                }

                p.rx() = upperPoint.x();
                p.ry() = upperPoint.y() + static_cast<int>(lineLength/2);
            }

            // Insert twice as these two points will form the new additional vertical or
            // horizontal line segment that is required to preserver straight angles.
            wire.insertPoint(1, p);
            wire.insertPoint(1, p);
        }
    }

    // Move the points
    for (int i = 0; i < wire.pointsRelative().count(); i++) {
        QPointF currPoint = wire.pointsRelative().at(i);
        if (currPoint == point-movedBy.toPoint()) {

            // Preserve straight angles (if supposed to)
            if (_settings.preserveStraightAngles) {

                // Move previous point
                if (i >= 1) {
                    QPointF prevPoint = wire.pointsRelative().at(i-1);
                    Line line(prevPoint, currPoint);

                    // Make sure that two wire points never collide
                    if (i >= 2 && Line(currPoint+movedBy.toPointF(), prevPoint).lenght() <= 2) {
                        wire.moveLineSegmentBy(i-2, movedBy);
                    }

                    // The line is horizontal
                    if (line.isHorizontal()) {
                        wire.movePointBy(i-1, QVector2D(0, movedBy.y()));
                    }

                    // The line is vertical
                    else if (line.isVertical()) {
                        wire.movePointBy(i-1, QVector2D(movedBy.x(), 0));
                    }
                }

                // Move next point
                if (i < wire.pointsRelative().count()-1) {
                    QPointF nextPoint = wire.pointsRelative().at(i+1);
                    Line line(currPoint, nextPoint);

                    // Make sure that two wire points never collide
                    if (Line(currPoint+movedBy.toPointF(), nextPoint).lenght() <= 2) {
                        wire.moveLineSegmentBy(i+1, movedBy);
                    }

                    // The line is horizontal
                    if (line.isHorizontal()) {
                        wire.movePointBy(i+1, QVector2D(0, movedBy.y()));
                    }

                    // The line is vertical
                    else if (line.isVertical()) {
                        wire.movePointBy(i+1, QVector2D(movedBy.x(), 0));
                    }
                }
            }

            // Move the actual point itself
            wire.movePointBy(i, movedBy);

            break;
        }
    }
}

QList<std::shared_ptr<Wire>> Scene::wiresConnectedTo(const Node& node, const QVector2D& offset) const
{
    QList<std::shared_ptr<Wire>> list;

    for (auto& wire : wires()) {
        for (const WirePoint& wirePoint : wire->wirePointsRelative()) {
            for (const QPointF& connectionPoint : node.connectionPoints()) {
                if (wirePoint == connectionPoint+offset.toPointF()) {
                    list.append(wire);
                    break;
                }
            }
        }
    }

    return list;
}

void Scene::showPopup(const Item& item)
{
    _popupInfobox.reset(addWidget(item.popupInfobox()));

    if (_popupInfobox) {
        _popupInfobox->setPos(_lastMousePos + QPointF(5, 5));
        _popupInfobox->setZValue(100);
    }
}

void Scene::addWireNet(std::unique_ptr<WireNet> wireNet)
{
    if (!wireNet) {
        return;
    }

    // Take ownership
    std::shared_ptr<WireNet> net(std::move(wireNet));

    connect(net.get(), &WireNet::pointMoved, this, &Scene::wirePointMoved);
    connect(net.get(), &WireNet::highlightChanged, this, &Scene::wireNetHighlightChanged);

    _nets.append(net);

    update();
}

QList<Item*> Scene::itemsAt(const QPointF& scenePos, Qt::SortOrder order) const
{
    QList<Item*> list;

    for (auto& graphicsItem : QGraphicsScene::items(scenePos, Qt::IntersectsItemShape, order)) {
        Item* item = qgraphicsitem_cast<Item*>(graphicsItem);
        if (item) {
            list << item;
        }
    }

    return list;
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    switch (_mode) {
    case NormalMode:
    {
        // Reset stuff
        _newWire.reset();

        // Handle selections
        QGraphicsScene::mousePressEvent(event);

        break;
    }

    case WireMode:
    {

        // Left mouse button
        if (event->button() == Qt::LeftButton) {

            // Start a new wire if there isn't already one. Else continue the current one.
            if (!_newWire) {
                if (_wireFactory) {
                    _newWire.reset(_wireFactory().release());
                } else {
                    _newWire = std::make_shared<Wire>();
                }
                _undoStack->push(new CommandItemAdd(this, _newWire));
            }

            // Snap to grid
            const QPointF& snappedPos = _settings.snapToGrid(event->scenePos());
            _newWire->appendPoint(snappedPos);
            _newWireSegment = true;
        }

        break;
    }
    }

    _lastMousePos = event->scenePos();
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    switch (_mode) {
    case NormalMode:
    {
        QGraphicsScene::mouseReleaseEvent(event);
        break;
    }

    case WireMode:
    {
        // Right mouse button: Abort wire mode
        if (event->button() == Qt::RightButton) {

            // Change the mode back to NormalMode if nothing below cursor
            if (QGraphicsScene::items(event->scenePos()).isEmpty()) {
                setMode(NormalMode);
            }

            // Show the context menu stuff
            QGraphicsScene::mouseReleaseEvent(event);
        }

        break;
    }
    }

    _lastMousePos = event->lastScenePos();
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    // Get rid of any popup infobox
    _popupInfobox.reset();

    // Retrieve the new mouse position
    QPointF newMousePos = event->scenePos();
    QVector2D movedBy(event->scenePos() - event->lastScenePos());

    switch (_mode) {

    case NormalMode:
    {
        // Move or resize if supposed to
        if (event->buttons() & Qt::LeftButton) {
            // Figure out if we're currently resizing something
            bool resizingNode = false;
            for (auto item : selectedItems()) {
                Node* node = qgraphicsitem_cast<Node*>(item.get());
                if (node && node->mode() == Node::Resize) {
                    resizingNode = true;
                    break;
                }
            }

            // Move
            if (!resizingNode) {

                // Create a list of selected items
                QVector<QPointer<Item>> itemsToMove;
                for (auto& i : selectedItems()) {
                    Item* item = qgraphicsitem_cast<Item*>(i.get());
                    if (item and item->isMovable()) {
                        itemsToMove << item;
                    }
                }

                // Perform the move
                if (!itemsToMove.isEmpty()) {
                    QVector2D moveBy(event->scenePos() - event->lastScenePos());
                    _undoStack->push(new CommandItemMove(itemsToMove, moveBy));
                }

            // Resize (and everything else)
            } else {

                QGraphicsScene::mouseMoveEvent(event);

            }
        }

        QGraphicsScene::mouseMoveEvent(event);

        break;
    }

    case WireMode:
    {
        // Make sure that there's a wire
        if (!_newWire) {
            break;
        }

        // Transform mouse coordinates to grid positions (snapped to nearest grid point)
        const QPointF& snappedPos = _settings.snapToGrid(event->scenePos());

        // Add a new wire segment. Only allow straight angles (if supposed to)
        if (_settings.routeStraightAngles) {
            if (_newWireSegment) {
                // Remove the last point if there was a previous segment
                if (_newWire->pointsRelative().count() > 1) {
                    _newWire->removeLastPoint();
                }

                // Create the intermediate point that creates the straight angle
                WirePoint prevNode(_newWire->pointsRelative().at(_newWire->pointsRelative().count()-1));
                QPointF corner(prevNode.x(), snappedPos.y());
                if (_invertWirePosture) {
                    corner.setX(snappedPos.x());
                    corner.setY(prevNode.y());
                }

                // Add the two new points
                _newWire->appendPoint(corner);
                _newWire->appendPoint(snappedPos);

                _newWireSegment = false;
            } else {
                // Create the intermediate point that creates the straight angle
                WirePoint p1(_newWire->pointsRelative().at(_newWire->pointsRelative().count()-3));
                QPointF p2(p1.x(), snappedPos.y());
                QPointF p3(snappedPos);
                if (_invertWirePosture) {
                    p2.setX(p3.x());
                    p2.setY(p1.y());
                }

                // Modify the actual wire
                _newWire->movePointTo(_newWire->pointsRelative().count()-2, p2);
                _newWire->movePointTo(_newWire->pointsRelative().count()-1, p3);
            }
        } else {
            // Don't care about angles and stuff. Fuck geometry, right?
            QPoint newGridPoint = _settings.toGridPoint(newMousePos);
            if (_newWire->pointsRelative().count() > 1) {
                _newWire->movePointTo(_newWire->pointsRelative().count()-1, newGridPoint);
            } else {
                _newWire->appendPoint(newGridPoint);
            }
        }

        break;
    }

    }

    // Save the last mouse position
    _lastMousePos = newMousePos;
}

void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    switch (_mode) {
    case NormalMode:
    {
        QGraphicsScene::mouseDoubleClickEvent(event);
        return;
    }

    case WireMode:
    {

        // Only do something if there's a wire
        if (_newWire && _newWire->pointsRelative().count() > 1) {
            bool wireIsFloating = true;

            // Get rid of the last point as mouseDoubleClickEvent() is following mousePressEvent()
            _newWire->removeLastPoint();

            // Check whether the wire was connected to a connector
            for (const QPointF& connectionPoint : connectionPoints()) {
                if (connectionPoint == _newWire->pointsRelative().last()) {
                    wireIsFloating = false;
                    break;
                }
            }

            // Check wether the wire was connected to another wire
            for (const auto& wire : wires()) {
                if (wire->pointIsOnWire(_newWire->pointsRelative().last())) {
                    wireIsFloating = false;
                    break;
                }
            }

            // Notify the user if the wire ended up on a non-valid thingy
            if (wireIsFloating) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Wire mode");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText("A wire must end on either:\n"
                               "  + A node connector\n"
                               "  + A wire\n");
                msgBox.exec();

                _newWire->removeLastPoint();

                return;
            }

            // Finish the current wire
            _newWire->setAcceptHoverEvents(true);
            _newWire->setFlag(QGraphicsItem::ItemIsSelectable, true);
            _newWire->simplify();
            connect(_newWire.get(), &Wire::showPopup, this, &Scene::showPopup);
            _newWire.reset();

            return;
        }

        return;
    }

    }
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    // Create a list of mime formats we can handle
    QStringList mimeFormatsWeCanHandle {
        MIME_TYPE_NODE,
    };

    // Check whether we can handle this drag/drop
    for (const QString& format : mimeFormatsWeCanHandle) {
        if (event->mimeData()->hasFormat(format)) {
            clearSelection();
            event->acceptProposedAction();
            return;
        }
    }

    event->ignore();
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->acceptProposedAction();
}

void Scene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->acceptProposedAction();
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();

    // Check the mime data
    const QMimeData* mimeData = event->mimeData();
    if (!mimeData) {
        return;
    }

    // Nodes
    if (mimeData->hasFormat(MIME_TYPE_NODE)) {
        // Get the ItemMimeData
        const ItemMimeData* mimeData = qobject_cast<const ItemMimeData*>(event->mimeData());
        if (!mimeData) {
            return;
        }

        // Get the Item
        auto item = mimeData->item();
        if (!item) {
            return;
        }

        // Add to the scene
        item->setPos(event->scenePos());
        _undoStack->push(new CommandItemAdd(this, std::move(item)));
    }
}

void Scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->drawPixmap(rect.topLeft(), _backgroundPixmap);
}

void Scene::renderCachedBackground()
{
    // Create the pixmap
    QRect rect = sceneRect().toRect();
    if (rect.isNull() or !rect.isValid()) {
        return;
    }
    QPixmap pixmap(rect.width(), rect.height());

    // Grid pen
    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(Qt::gray);
    gridPen.setCapStyle(Qt::RoundCap);
    gridPen.setWidth(_settings.gridPointSize);

    // Grid brush
    QBrush gridBrush;
    gridBrush.setStyle(Qt::NoBrush);

    // Create a painter
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, _settings.antialiasing);

    // Draw background
    pixmap.fill(Qt::white);

    // Draw the grid if supposed to
    if (_settings.showGrid and (_settings.gridSize > 0)) {
        qreal left = int(rect.left()) - (int(rect.left()) % _settings.gridSize);
        qreal top = int(rect.top()) - (int(rect.top()) % _settings.gridSize);

        // Create a list of points
        QVector<QPointF> points;
        for (qreal x = left; x < rect.right(); x += _settings.gridSize) {
            for (qreal y = top; y < rect.bottom(); y += _settings.gridSize) {
                points.append(QPointF(x,y));
            }
        }

        // Draw the actual grid points
        painter.setPen(gridPen);
        painter.setBrush(gridBrush);
        painter.drawPoints(points.data(), points.size());
    }

    // Mark the origin if supposed to
    if (_settings.debug) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(Qt::red));
        painter.drawEllipse(-6, -6, 12, 12);
    }

    painter.end();

    // Update
    _backgroundPixmap = pixmap;
    update();
}

void Scene::setupNewItem(Item& item)
{
    // Set settings
    item.setSettings(_settings);

    // Connections
    connect(&item, &Item::moved, this, &Scene::itemMoved);
    connect(&item, &Item::showPopup, this, &Scene::showPopup);
}

QList<QPointF> Scene::connectionPoints() const
{
    QList<QPointF> list;

    for (const auto& node : nodes()) {
        for (const auto& connectionPoint : node->connectionPoints()) {
            list << connectionPoint + node->pos();
        }
    }

    return list;
}
