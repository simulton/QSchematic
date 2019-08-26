#include <algorithm>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QUndoStack>
#include <QPixmap>
#include <QMimeData>
#include <QtMath>
#include "scene.h"
#include "commands/commanditemmove.h"
#include "commands/commanditemadd.h"
#include "items/itemfactory.h"
#include "items/item.h"
#include "items/itemmimedata.h"
#include "items/node.h"
#include "items/wire.h"
#include "items/wirenet.h"

using namespace QSchematic;

Scene::Scene(QObject* parent) :
    QGraphicsScene(parent),
    _mode(NormalMode),
    _newWireSegment(false),
    _invertWirePosture(true),
    _movingNodes(false)
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

Gpds::Container Scene::toContainer() const
{
    // Scene
    Gpds::Container scene;
    {
        // Rect
        Gpds::Container r;
        const QRect& rect = sceneRect().toRect();
        r.addValue("x", rect.x());
        r.addValue("y", rect.y());
        r.addValue("width", rect.width());
        r.addValue("height", rect.height());
        scene.addValue("rect", r);
    }

    // Nodes
    Gpds::Container nodesList;
    for (const auto& node : nodes()) {
        nodesList.addValue("node", node->toContainer());
    }

    // Nets
    Gpds::Container netsList;
    for (const auto& net : nets()) {
        netsList.addValue("net", net->toContainer());
    }

    // Root
    Gpds::Container c;
    c.addValue("scene", scene);
    c.addValue("nodes", nodesList);
    c.addValue("nets", netsList);

    return c;
}

void Scene::fromContainer(const Gpds::Container& container)
{
    // Scene
    {
        const Gpds::Container* sceneContainer = container.getValue<Gpds::Container*>("scene");
        Q_ASSERT( sceneContainer );

        // Rect
        const Gpds::Container* rectContainer = sceneContainer->getValue<Gpds::Container*>("rect");
        if ( rectContainer ) {
            QRect rect;
            rect.setX( rectContainer->getValue<int>("x") );
            rect.setY( rectContainer->getValue<int>("y") );
            rect.setWidth( rectContainer->getValue<int>("width") );
            rect.setHeight( rectContainer->getValue<int>("height") );

            setSceneRect( rect );
        }
    }

    // Nodes
    const Gpds::Container* nodesContainer = container.getValue<Gpds::Container*>("nodes");
    if ( nodesContainer ) {
        for (const auto& nodeContainer : nodesContainer->getValues<Gpds::Container*>("node")) {
            Q_ASSERT(nodeContainer);

            std::unique_ptr<Item> node = ItemFactory::instance().fromContainer(*nodeContainer);
            if (!node) {
                qCritical("Scene::fromContainer(): Couldn't restore node. Skipping.");
                continue;
            }
            node->fromContainer(*nodeContainer);
            addItem(std::move(node));
        }
    }

    // Nets
    const Gpds::Container* netsContainer = container.getValue<Gpds::Container*>("nets");
    if ( netsContainer ) {
        Q_ASSERT( netsContainer );

        for (const Gpds::Container* netContainer : netsContainer->getValues<Gpds::Container*>("net")) {
            Q_ASSERT( netContainer );

            auto net = std::make_shared<WireNet>();
            net->fromContainer( *netContainer );

            for (auto& wire : net->wires()) {
                addItem(wire);
            }

            addWireNet(net);
        }
    }

    // Attach the wires to the nodes
    for (const auto& net: _nets) {
        for (const auto& wire: net->wires()) {
            for (const auto& node: nodes()) {
                for (const auto& connector: node->connectors()) {
                    for (const auto& point: wire->wirePointsAbsolute()) {
                        if (QVector2D(connector->scenePos() - point.toPointF()).length() < 1) {
                            connector->attachWire(wire.get(), wire->wirePointsAbsolute().indexOf(point));
                            break;
                        }
                    }
                }
            }
        }
    }

    // Clear the undo history
    _undoStack->clear();
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

void Scene::setMode(int mode)
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

int Scene::mode() const
{
    return _mode;
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
    while (!_items.isEmpty()) {
        removeItem(_items.first());
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

    // Remove from scene (if necessary)
    if (item->QGraphicsItem::scene()) {
        QGraphicsScene::removeItem(item.get());
    }

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

QList<std::shared_ptr<Item>> Scene::itemsAt(const QPointF &scenePos, Qt::SortOrder order) const
{
    QList<std::shared_ptr<Item>> items;

    for (auto& graphicsItem : QGraphicsScene::items(scenePos, Qt::IntersectsItemShape, order)) {
        Item* item = qgraphicsitem_cast<Item*>(graphicsItem);
        if (item) {
            auto sharedItem = sharedItemPointer( *item );
            items << sharedItem;
        }
    }

    return items;
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
            for (const QPointF& point : wire->pointsRelative()) {
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
}

void Scene::itemRotated(const Item& item, const qreal rotation)
{
}

void Scene::itemHighlightChanged(const Item& item, bool isHighlighted)
{
    // Retrieve the corresponding smart pointer
    auto sharedPointer = sharedItemPointer(item);
    if (not sharedPointer) {
        return;
    }

    // Let the world know
    emit itemHighlightChanged(sharedPointer, isHighlighted);
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

void Scene::wirePointMovedByUser(Wire& rawWire, WirePoint& point)
{
    int index = rawWire.wirePointsRelative().indexOf(point);
    // Detach from connector
    for (const auto& node: nodes()) {
        for (const auto& connector: node->connectors()) {
            const Wire* attachedWire = connector->attachedWire();
            if (not attachedWire) {
                continue;
            }

            if (attachedWire != &rawWire) {
                continue;
            }

            if (connector->attachedWirepoint() == index) {
                if (connector->scenePos().toPoint() != rawWire.pointsAbsolute().at(index).toPoint()) {
                    connector->detachWire();
                }
                return;
            }
        }
    }

    // Attach to connector
    for (const auto& node: nodes()) {
        for (const auto& connector: node->connectors()) {
            if (connector->scenePos().toPoint() == (rawWire.pos() + point.toPointF()).toPoint()) {
                connector->attachWire(&rawWire, rawWire.wirePointsRelative().indexOf(point));
            }
        }
    }
}

void Scene::addWireNet(const std::shared_ptr<WireNet>& wireNet)
{
    // Sanity check
    if (!wireNet) {
        return;
    }

    // Setup
    connect(wireNet.get(), &WireNet::pointMoved, this, &Scene::wirePointMoved);
    connect(wireNet.get(), &WireNet::pointMovedByUser, this, &Scene::wirePointMovedByUser);
    connect(wireNet.get(), &WireNet::highlightChanged, this, &Scene::wireNetHighlightChanged);

    // Keep track of stuff
    _nets.append(wireNet);
}

std::shared_ptr<Item> Scene::sharedItemPointer(const Item& item) const
{
    // Check for all known _items
    for (const auto& sharedPointer : _items) {
        if (sharedPointer.get() == &item) {
            return sharedPointer;
        }
    }

    // Check for connectors
    if (item.parentItem()) {
        const Node* node = qgraphicsitem_cast<const Node*>( item.parentItem() );
        if ( node ) {
            for ( const auto& connector : node->connectors() ) {
                if ( connector.get() == &item ) {
                    return connector;
                }
            }
        }
    }

    return nullptr;
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

        // Check if moving nodes
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (item){
            Node* node = dynamic_cast<Node*>(item);
            if (node && node->mode() != Node::None) {
                _movingNodes = false;
            } else {
                _movingNodes = true;
            }
        } else {
            _movingNodes = false;
        }

        // Store the initial position of all the selected items
        _initialItemPositions.clear();
        for (auto& item: selectedItems()) {
            if (item) {
                _initialItemPositions.insert(item, item->pos());
            }
        }

        // Store the initial cursor position
        _initialCursorPosition = event->scenePos();

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

            // Attach point to connector if needed
            for (const auto& node: nodes()) {
                for (const auto& connector: node->connectors()) {
                    if (QVector2D(connector->scenePos() - snappedPos).length() < 1) {
                        connector->attachWire(_newWire.get(), _newWire->pointsAbsolute().indexOf(snappedPos));
                        break;
                    }
                }
            }
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

        // Reset the position for every selected item and
        // apply the translation through the undostack
        if (_movingNodes) {
            for (auto& i: selectedItems()) {
                Item* item = qgraphicsitem_cast<Item*>(i.get());
                // Move the item if it is movable and it was previously registered by the mousePressEvent
                if (item and item->isMovable() and _initialItemPositions.contains(i)) {
                    QVector2D moveBy(item->pos() - _initialItemPositions.value(i));
                    if (!moveBy.isNull()) {
                        // Move the item to its initial position
                        item->setPos(_initialItemPositions.value(i));
                        // Apply the translation
                        _undoStack->push(new CommandItemMove(QVector<std::shared_ptr<Item>>() << i, moveBy));
                    }
                }
            }
        }
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

    // Retrieve the new mouse position
    QPointF newMousePos = event->scenePos();
    QVector2D movedBy(event->scenePos() - event->lastScenePos());

    switch (_mode) {

    case NormalMode:
    {
        // Let the base class handle the basic stuff
        // Note that we DO NOT want this in WireMode to prevent highlighting of the wires
        // during placing a new wire.
        QGraphicsScene::mouseMoveEvent(event);

        // Move, resize or rotate if supposed to
        if (event->buttons() & Qt::LeftButton) {
            // Move all selected items
            if (_movingNodes) {
                for (auto& i : selectedItems()) {
                    Item* item = qgraphicsitem_cast<Item*>(i.get());
                    if (item and item->isMovable()) {
                        // Calculate by how much the item was moved
                        QPointF moveBy = _initialItemPositions.value(i) + newMousePos - _initialCursorPosition - item->pos();
                        // Apply the custom scene snapping
                        moveBy = itemsMoveSnap(i, QVector2D(moveBy)).toPointF();
                        // Move the item
                        item->setPos(item->pos() + moveBy);
                    }
                }
            }
        }

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
            if (_newWire->pointsRelative().count() > 1) {
                _newWire->movePointTo(_newWire->pointsRelative().count()-1, snappedPos);
            } else {
                _newWire->appendPoint(snappedPos);
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
    const QPointF& pixmapTopleft = rect.topLeft() - sceneRect().topLeft();

    painter->drawPixmap(rect, _backgroundPixmap, QRectF(pixmapTopleft.x(), pixmapTopleft.y(), rect.width(), rect.height()));
}

QVector2D Scene::itemsMoveSnap(const std::shared_ptr<Item>& items, const QVector2D& moveBy) const
{
    Q_UNUSED(items);

    return moveBy;
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
    connect(&item, &Item::rotated, this, &Scene::itemRotated);
}

QList<QPointF> Scene::connectionPoints() const
{
    QList<QPointF> list;

    for (const auto& node : nodes()) {
        list << node->connectionPointsAbsolute();
    }

    return list;
}
