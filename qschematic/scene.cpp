#include <algorithm>

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QUndoStack>
#include <QMimeData>
#include <QtMath>
#include <QTimer>

#include "scene.h"
#include "commands/commanditemmove.h"
#include "commands/commanditemadd.h"
#include "commands/commanditemremove.h"
#include "items/itemfactory.h"
#include "items/item.h"
#include "items/itemmimedata.h"
#include "items/node.h"
#include "items/label.h"
#include "items/widget.h"
#include "utils/itemscontainerutils.h"

using namespace QSchematic;

Scene::Scene(QObject* parent) :
    QGraphicsScene(parent),
    _mode(NormalMode),
    _newWireSegment(false),
    _invertWirePosture(true),
    _movingNodes(false),
    _highlightedItem(nullptr)
{
    // NOTE: still needed, BSP-indexer still crashes on a scene load when
    // the scene is already populated
    setItemIndexMethod(ItemIndexMethod::NoIndex);

    // Wire system
    m_wire_manager = std::make_shared<wire_system::manager>();
    m_wire_manager->set_net_factory([=] { return std::make_shared<WireNet>(); });
    connect(m_wire_manager.get(), &wire_system::manager::wire_point_moved, this, &Scene::wirePointMoved);

    // Undo stack
    _undoStack = new QUndoStack(this);
    connect(_undoStack, &QUndoStack::cleanChanged, [this](bool isClean) {
        emit isDirtyChanged(!isClean);
    });

    // Popup timer
    _popupTimer = new QTimer(this);
    _popupTimer->setSingleShot(true);
    connect(_popupTimer, &QTimer::timeout, [this]{
        // Sanity check
        if (!_highlightedItem)
            return;

        // Get item popup
        _popup.reset( QGraphicsScene::addWidget(_highlightedItem->popup().release()) );
        _popup->setZValue(100);
        _popup->setPos(_lastMousePos + QPointF{ 5, 5 });
    });

    // Stuff
    connect(this, &QGraphicsScene::sceneRectChanged, [this]{
        renderCachedBackground();
    });

    // Prepare the background
    renderCachedBackground();
}

gpds::container Scene::to_container() const
{
    // Scene
    gpds::container scene;
    {
        // Rect
        gpds::container r;
        const QRect& rect = sceneRect().toRect();
        r.add_value("x", rect.x());
        r.add_value("y", rect.y());
        r.add_value("width", rect.width());
        r.add_value("height", rect.height());
        scene.add_value("rect", r);
    }

    // Nodes
    gpds::container nodesList;
    for (const auto& node : nodes()) {
        nodesList.add_value("node", node->to_container());
    }

    // Nets
    gpds::container netsList;
    for (const auto& net : m_wire_manager->nets()) {

        // Make sure it's a WireNet
        auto wire_net = std::dynamic_pointer_cast<WireNet>(net);
        if (!wire_net) {
            continue;
        }

        netsList.add_value("net", wire_net->to_container());
    }

    // Root
    gpds::container c;
    c.add_value("scene", scene);
    c.add_value("nodes", nodesList);
    c.add_value("nets", netsList);

    return c;
}

void Scene::from_container(const gpds::container& container)
{
    // Scene
    {
        const gpds::container* sceneContainer = container.get_value<gpds::container*>("scene").value_or(nullptr);
        Q_ASSERT( sceneContainer );

        // Rect
        const gpds::container* rectContainer = sceneContainer->get_value<gpds::container*>("rect").value_or(nullptr);
        if ( rectContainer ) {
            QRect rect;
            rect.setX(rectContainer->get_value<int>("x").value_or(0));
            rect.setY(rectContainer->get_value<int>("y").value_or(0));
            rect.setWidth(rectContainer->get_value<int>("width").value_or(0));
            rect.setHeight(rectContainer->get_value<int>("height").value_or(0));

            setSceneRect( rect );
        }
    }

    // Nodes
    const gpds::container* nodesContainer = container.get_value<gpds::container*>("nodes").value_or(nullptr);
    if ( nodesContainer ) {
        for (const auto& nodeContainer : nodesContainer->get_values<gpds::container*>("node")) {
            Q_ASSERT(nodeContainer);

            auto node = ItemFactory::instance().from_container(*nodeContainer);
            if (!node) {
                qWarning("Scene::from_container(): Couldn't restore node. Skipping.");
                continue;
            }
            node->from_container(*nodeContainer);
            addItem(node);
        }
    }

    // Nets
    const gpds::container* netsContainer = container.get_value<gpds::container*>("nets").value_or(nullptr);
    if ( netsContainer ) {
        Q_ASSERT( netsContainer );

        for (const gpds::container* netContainer : netsContainer->get_values<gpds::container*>("net")) {
            Q_ASSERT( netContainer );

            auto net = std::make_shared<WireNet>();
            net->setScene(this);
            net->set_manager(wire_manager().get());
            net->from_container( *netContainer );

            m_wire_manager->add_net(net);
        }
    }

    // Attach the wires to the nodes
    generateConnections();

    // Find junctions
    m_wire_manager->generate_junctions();

    // Clear the undo history
    _undoStack->clear();
}

void Scene::setSettings(const Settings& settings)
{
    // Update settings of all items
    for (auto& item : items()) {
        item->setSettings(settings);
    }

    // Update settings of the wire manager
    m_wire_manager->set_settings(settings);

    // Store new settings
    _settings = settings;

    // Redraw
    renderCachedBackground();
    update();
}

void Scene::setWireFactory(const std::function<std::shared_ptr<Wire>()>& factory)
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
        if (_newWire) {
            _newWire->simplify();
        }
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
    // Ensure no lingering lifespans kept in map-keys, selections or undocommands
    _initialItemPositions.clear();
    clearSelection();
    clearFocus();
    _undoStack->clear();

    // Remove from scene
    // Do not use QGraphicsScene::clear() as that would also delete the items. However,
    // we still need them as we manage them via smart pointers (eg. in commands)
    while (!_items.isEmpty()) {
        removeItem(_items.first());
    }

    // Nets
    m_wire_manager->clear();

    // Now that all the top-level items are safeguarded we can call the underlying scene's clear()
    QGraphicsScene::clear();

    // NO longer dirty
    clearIsDirty();
}

/**
 * Adds an item to the scene
 * \remark When adding an Item to the Scene, there are two possibilities. If the
 * Item is a "Top-Level" Item (it has no parent), then you should use
 * Scene::addItem(). If the Item is the child of another Item then you need to
 * use the superclass' implementation QGraphicsItem::addItem().
 * This is needed to determine which Items should be moved by the scene and which
 * are moved by their parent.
 */
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

bool Scene::removeItem(const std::shared_ptr<Item> item)
{
    // Sanity check
    if (!item)
        return false;

    // Figure out what area we need to update
    auto itemBoundsToUpdate = item->mapRectToScene(item->boundingRect());

    // NOTE: Sometimes ghosts remain (not drawn away) when they're active in some way at remove time, found below from looking at Qt-source code...
    item->clearFocus();
    item->setFocusProxy(nullptr);

    // Remove from scene (if necessary)
    QGraphicsScene::removeItem(item.get());

    // Remove shared pointer from local list to reduce instance count
    _items.removeAll(item);

    // Update the corresponding scene area (redraw)
    update(itemBoundsToUpdate);

    // Let the world know
    emit itemRemoved(item);

    // NOTE: In order to keep items alive through this entire event loop round,
    // otherwise crashes because Qt messes with items even after they're removed
    // ToDo: Fix this
    _keep_alive_an_event_loop << item;

    return true;
}

QList<std::shared_ptr<Item>> Scene::items() const
{
    return _items;
}

QList<std::shared_ptr<Item>> Scene::itemsAt(const QPointF &scenePos, Qt::SortOrder order) const
{
    return ItemUtils::mapItemListToSharedPtrList<QList>(QGraphicsScene::items(scenePos, Qt::IntersectsItemShape, order));
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

std::vector<std::shared_ptr<Item>> Scene::selectedItems() const
{

    // 111
    // auto items = ItemUtils::mapItemListToSharedPtrList<std::vector>(QGraphicsScene::selectedItems());


    // 222
    // const auto& rawItems = QGraphicsScene::selectedItems();
    // // Retrieve corresponding smart pointers
    // auto items = std::vector<std::shared_ptr<Item>>{};
    // items.reserve(rawItems.count());
    // for ( auto item_ptr : rawItems ) {
    //     if ( auto qs_item = dynamic_cast<Item*>(item_ptr) ) {
    //         if ( auto item_sh_ptr = qs_item->sharedPtr() ) {
    //             items.push_back(item_sh_ptr );
    //         }
    //     }
    // }

    // ???
    // TODO: XXX
    // - get items but sort out only those in root(?)
    // - this will break for group style (ExdDes)
    // - but QS-demo behaves wickedly if child-items are allowed to be selected since it didn't support that before

    auto items = ItemUtils::mapItemListToSharedPtrList<std::vector>(QGraphicsScene::selectedItems());

    return items;
}

/**
 * Returns only the selected items that are not part of another item.
 * \remark The top-level items are those that were added to the scene by calling Scene::addItem.
 * Items that should not be top-level items need to be added by using QGraphicsScene::addItem
 * \returns List of selected top-level items
 */
std::vector<std::shared_ptr<Item>> Scene::selectedTopLevelItems() const
{
    const auto& rawItems = QGraphicsScene::selectedItems();
    std::vector<std::shared_ptr<Item>> items;

    for (auto& item : _items) {
        if (rawItems.contains(item.get())) {
            items.push_back(item);
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

std::shared_ptr<Node> Scene::nodeFromConnector(const QSchematic::Connector& connector) const
{
    for (auto node : nodes()) {
        const auto& connectors = node->connectors();
        auto it = std::find_if(std::cbegin(connectors), std::cend(connectors), [&connector](const auto& c) {
            return c.get() == &connector;
        });
        if (it != std::cend(connectors))
            return node;
    }

    return nullptr;
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

std::shared_ptr<wire_system::manager> Scene::wire_manager() const
{
    return m_wire_manager;
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    // Remove popup
    _popupTimer->stop();
    _popup = { };

    switch (_mode) {
    case NormalMode:
    {
        // Reset stuff
        _newWire = {};

        // Handle selections
        QGraphicsScene::mousePressEvent(event);

        // Check if moving nodes
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (item){
            Node* node = dynamic_cast<Node*>(item);
            if (node && node->mode() == Node::None) {
                _movingNodes = true;
            } else {
                _movingNodes = false;
            }
            // Prevent the scene from detecting changes in the wires origin
            // when the bouding rect is resized by a wire_point that moved
            Wire* wire = dynamic_cast<Wire*>(item);
            if (wire) {
                if (wire->movingWirePoint()) {
                    _movingNodes = false;
                } else {
                    _movingNodes = true;
                }
            }
            Label* label = dynamic_cast<Label*>(item);
            if (label && selectedTopLevelItems().size() > 0) {
                _movingNodes = true;
            }
        } else {
            _movingNodes = false;
        }

        // Store the initial position of all the selected items
        _initialItemPositions.clear();
        for (auto& item: selectedTopLevelItems()) {
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
                _newWire = make_wire();
                _undoStack->push(new CommandItemAdd(this, _newWire));
                _newWire->setPos(_settings.snapToGrid(event->scenePos()));
            }
            // Snap to grid
            const QPointF& snappedPos = _settings.snapToGrid(event->scenePos());
            _newWire->append_point(snappedPos);
            _newWireSegment = true;

            // Attach point to connector if needed
            bool wireAttached = false;
            for (const auto& node: nodes()) {
                for (const auto& connector: node->connectors()) {
                    // Ignore hidden connectors
                    if (!connector->isVisible())
                        continue;

                    if (QVector2D(connector->scenePos() - snappedPos).length() < 1) {
                        m_wire_manager->attach_wire_to_connector(_newWire.get(), _newWire->pointsAbsolute().indexOf(snappedPos),
                                                                 connector.get());
                        wireAttached = true;
                        break;
                    }
                }
            }

            // Attach point to wire if needed
            for (const auto& wire: m_wire_manager->wires()) {
                // Skip current wire
                if (wire == _newWire) {
                    continue;
                }
                if (wire->point_is_on_wire(_newWire->pointsAbsolute().last())) {
                    m_wire_manager->connect_wire(wire.get(), _newWire.get(), _newWire->pointsAbsolute().count() - 1);
                    wireAttached = true;
                    break;
                }
            }

            // Check if both ends of the wire are connected to something
            if (wireAttached && _newWire->pointsAbsolute().count() > 1) {
                finishCurrentWire();
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

        for (const auto& net : m_wire_manager->nets()) {

            // Make sure it's a WireNet
            auto wire_net = std::dynamic_pointer_cast<WireNet>(net);
            if (!wire_net) {
                continue;
            }

            wire_net->updateLabelPos(true);
        }
        // Reset the position for every selected item and
        // apply the translation through the undostack
        if (_movingNodes) {
            QVector<std::shared_ptr<Item>> wiresToMove;
            QVector<std::shared_ptr<Item>> itemsToMove;

            for (const auto& item : selectedTopLevelItems()) {
                if (item->isMovable() && _initialItemPositions.contains(item)) {
                    Wire* wire = dynamic_cast<Wire*>(item.get());
                    if (wire) {
                        wiresToMove << item;
                    } else {
                        itemsToMove << item;
                    }
                }
            }

            itemsToMove = wiresToMove << itemsToMove;
            bool needsToMove = false;
            QVector<QVector2D> moveByList;

            for (const auto& item : itemsToMove) {
                // Move the item if it is movable and it was previously registered by the mousePressEvent
                QVector2D moveBy(item->pos() - _initialItemPositions.value(item));
                // Move the item to its initial position
                item->setPos(_initialItemPositions.value(item));
                // Add the moveBy to the list
                moveByList << moveBy;
                if (!moveBy.isNull()) {
                    needsToMove = true;
                }
            }
            // Apply the translation
            if (needsToMove) {
                _undoStack->push(new CommandItemMove(itemsToMove, moveByList));
            }
            for (const auto& item : itemsToMove) {
                const Node* node = dynamic_cast<const Node*>(item.get());
                if (node) {
                    updateNodeConnections(node);
                }
            }

            for (auto& item : m_wire_manager->wires()) {
                Wire* wire = dynamic_cast<Wire*>(item.get());
                if (wire) {
//                    wire->updatePosition();
                    wire->simplify();
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

void Scene::updateNodeConnections(const Node* node) const
{
    // Check if a connector lays on a wirepoint
    for (auto& connector : node->connectors()) {
        // Skip hidden connectors
        if (!connector->isVisible())
            continue;
        
        // If the connector already has a wire attached, skip
        if (m_wire_manager->attached_wire(connector.get()) != nullptr) {
            continue;
        }
        // Find if there is a point to connect to
        for (const auto& wire : m_wire_manager->wires()) {
            int index = -1;
            if (wire->points().first().toPoint() == connector->scenePos().toPoint()) {
                index = 0;
            } else if (wire->points().last().toPoint() == connector->scenePos().toPoint()) {
                index = wire->points().count() - 1;
            }
            if (index != -1) {
                // Ignore if it's a junction
                if (wire->points().at(index).is_junction()){
                    continue;
                }
                // Check if it isn't already connected to another connector
                bool alreadyConnected = false;
                for (const auto& otherConnector : connectors()) {
                    if (otherConnector == connector) {
                        continue;
                    }
                    if (m_wire_manager->attached_wire(connector.get()) == wire.get() &&
                        m_wire_manager->attached_point(otherConnector.get()) == index) {
                        alreadyConnected = true;
                        break;
                    }
                }
                // If it's not already connected, connect it
                if (!alreadyConnected) {
                    m_wire_manager->attach_wire_to_connector(wire.get(), index, connector.get());
                }
            }
        }
    }
}

void Scene::wirePointMoved(wire& rawWire, int index)
{
    // Detach from connector
    for (const auto& node: nodes()) {
        for (const auto& connector: node->connectors()) {
            const wire* wire = m_wire_manager->attached_wire(connector.get());
            if (!wire) {
                continue;
            }

            if (wire != &rawWire) {
                continue;
            }

            if (m_wire_manager->attached_point(connector.get()) == index) {
                if (connector->scenePos().toPoint() != rawWire.points().at(index).toPoint()) {
                    m_wire_manager->detach_wire(connector.get());
                }
            }
        }
    }

    // Attach to connector
    point point = rawWire.points().at(index);
    for (const auto& node: nodes()) {
        for (const auto& connector: node->connectors()) {
            if (connector->scenePos().toPoint() == point.toPoint()) {
                m_wire_manager->attach_wire_to_connector(&rawWire, index, connector.get());
            }
        }
    }
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    // Retrieve the new mouse position
    QPointF newMousePos = event->scenePos();

    switch (_mode) {

    case NormalMode:
    {
        // Let the base class handle the basic stuff
        // Note that we DO NOT want this in WireMode to prevent highlighting of the wires
        // during placing a new wire.

        // Move, resize or rotate if supposed to
        if (event->buttons() & Qt::LeftButton) {
            // Move all selected items
            if (_movingNodes) {
                QVector<std::shared_ptr<Item>> wiresToMove;
                QVector<std::shared_ptr<Item>> itemsToMove;
                for (const auto& item : selectedTopLevelItems()) {
                    if (item->isMovable()) {
                        Wire* wire = dynamic_cast<Wire*>(item.get());
                        if (wire) {
                            wiresToMove << item;
                        } else {
                            itemsToMove << item;
                        }
                    }
                }
                itemsToMove = wiresToMove << itemsToMove;
                for (const auto& item : itemsToMove) {
                    // Calculate by how much the item was moved
                    QPointF moveBy = _initialItemPositions.value(item) + newMousePos - _initialCursorPosition - item->pos();
                    // Apply the custom scene snapping
                    moveBy = itemsMoveSnap(item, QVector2D(moveBy)).toPointF();
                    item->setPos(item->pos() + moveBy);
                }
                // Simplify all the wires
                for (auto& wire : m_wire_manager->wires()) {
                    wire->simplify();
                }
            }
            else {
                QGraphicsScene::mouseMoveEvent(event);
            }
        } else {
            QGraphicsScene::mouseMoveEvent(event);
        }

        // Highlight the item under the cursor
        Item* item = dynamic_cast<Item*>(itemAt(newMousePos, QTransform()));
        if (item && item->highlightEnabled()) {
            // Skip if the item is already highlighted
            if (item == _highlightedItem) {
                break;
            }
            // Disable the highlighting on the previous item
            if (_highlightedItem) {
                _highlightedItem->setHighlighted(false);
                itemHoverLeave(_highlightedItem->shared_from_this());
                _highlightedItem->update();
                emit _highlightedItem->highlightChanged(*_highlightedItem, false);
                _highlightedItem = nullptr;
            }
            // Highlight the item
            item->setHighlighted(true);
            itemHoverEnter(item->shared_from_this());
            item->update();
            emit item->highlightChanged(*item, true);
            _highlightedItem = item;
        }
        // No item selected
        else if (_highlightedItem) {
            _highlightedItem->setHighlighted(false);
            itemHoverLeave(_highlightedItem->shared_from_this());
            _highlightedItem->update();
            emit _highlightedItem->highlightChanged(*_highlightedItem, false);
            _highlightedItem = nullptr;
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
                point prevNode(_newWire->pointsAbsolute().at(_newWire->pointsAbsolute().count() - 1));
                QPointF corner(prevNode.x(), snappedPos.y());
                if (_invertWirePosture) {
                    corner.setX(snappedPos.x());
                    corner.setY(prevNode.y());
                }

                // Add the two new points
                _newWire->append_point(corner);
                _newWire->append_point(snappedPos);

                _newWireSegment = false;
            } else {
                // Create the intermediate point that creates the straight angle
                point p1(_newWire->pointsAbsolute().at(_newWire->pointsAbsolute().count() - 3));
                QPointF p2(p1.x(), snappedPos.y());
                QPointF p3(snappedPos);
                if (_invertWirePosture) {
                    p2.setX(p3.x());
                    p2.setY(p1.y());
                }

                // Modify the actual wire
                _newWire->move_point_to(_newWire->pointsAbsolute().count() - 2, p2);
                _newWire->move_point_to(_newWire->pointsAbsolute().count() - 1, p3);
            }
        } else {
            // Don't care about angles and stuff. Fuck geometry, right?
            if (_newWire->pointsAbsolute().count() > 1) {
                _newWire->move_point_to(_newWire->pointsAbsolute().count() - 1, snappedPos);
            } else {
                _newWire->append_point(snappedPos);
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

    // Remove popup
    _popupTimer->stop();
    _popup = { };

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

            // Get rid of the last point as mouseDoubleClickEvent() is following mousePressEvent()
            _newWire->removeLastPoint();

            // Attach point to wire if needed
            for (const auto& wire: m_wire_manager->wires()) {
                // Skip current wire
                if (wire == _newWire) {
                    continue;
                }
                if (wire->point_is_on_wire(_newWire->pointsAbsolute().last())) {
                    m_wire_manager->connect_wire(wire.get(), _newWire.get(), _newWire->pointsAbsolute().count() - 1);
                }
            }

            // Finish the current wire
            finishCurrentWire();

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

QPixmap Scene::renderBackground(const QRect& rect) const
{
    // Create pixmap
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
    if (_settings.showGrid && (_settings.gridSize > 0)) {
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

    return pixmap;
}

void Scene::renderCachedBackground()
{
    // Create the pixmap
    const QRect& rect = sceneRect().toRect();
    if (rect.isNull() || !rect.isValid())
        return;

    // Render background
    _backgroundPixmap = std::move(renderBackground(rect));

    // Update
    update();
}

void Scene::setupNewItem(Item& item)
{
    // Set settings
    item.setSettings(_settings);
}

void Scene::generateConnections()
{
    for (const auto& connector : connectors()) {
        std::shared_ptr<wire> wire = m_wire_manager->wire_with_extremity_at(connector->scenePos());
        if (wire) {
            m_wire_manager->attach_wire_to_connector(wire.get(), connector.get());
        }
    }
}

/**
 * Finishes the current wire if there is one
 */
void Scene::finishCurrentWire()
{
    if (!_newWire) {
        return;
    }
    // Finish the current wire
    _newWire->setAcceptHoverEvents(true);
    _newWire->setFlag(QGraphicsItem::ItemIsSelectable, true);
    _newWire->simplify();
//    _newWire->updatePosition();
    _newWire.reset();
}

std::shared_ptr<Wire>
Scene::make_wire() const
{
    if (_wireFactory)
        return _wireFactory();
    else
        return std::make_shared<Wire>();
}

QList<QPointF> Scene::connectionPoints() const
{
    QList<QPointF> list;

    for (const auto& node : nodes()) {
        list << node->connectionPointsAbsolute();
    }

    return list;
}

QList<std::shared_ptr<Connector>> Scene::connectors() const
{
    QList<std::shared_ptr<Connector>> list;

    for (const auto& node : nodes()) {
        list << node->connectors();
    }

    return list;
}

void Scene::itemHoverEnter(const std::shared_ptr<const Item>& item)
{
    emit itemHighlighted(item);

    // Start popup timer
    _popupTimer->start(_settings.popupDelay);
}

void Scene::itemHoverLeave([[maybe_unused]] const std::shared_ptr<const Item>& item)
{
    // ToDo: clear _highlightedItem ?

    emit itemHighlighted(nullptr);

    // Stop popup timer
    _popupTimer->stop();
    _popup = { };
}

/**
 * Removes the last point(s) of the new wire. After execution, the wire should
 * be in the same state it was before the last point had been added.
 */
void Scene::removeLastWirePoint()
{
    if (!_newWire) {
        return;
    }

    // If we're supposed to preseve right angles, two points have to be removed
    if (_settings.routeStraightAngles) {
        // Do nothing if there are not at least 4 points
        if (_newWire->pointsAbsolute().count() > 3) {
            // Keep the position of the last point
            QPointF mousePos = _newWire->pointsAbsolute().last();
            // Remove both points
            _newWire->removeLastPoint();
            _newWire->removeLastPoint();
            // Move the new last point to where the previous last point was
            _newWire->move_point_by(_newWire->pointsAbsolute().count() - 1,
                                    QVector2D(mousePos - _newWire->pointsAbsolute().last()));
        }
    }

    // If we don't care about the angles, only the last point has to be removed
    else {
        // Do nothing if there are not at least 3 points
        if (_newWire->pointsAbsolute().count() > 2) {
            // Keep the position of the last point
            QPointF mousePos = _newWire->pointsAbsolute().last();
            // Remove the point
            _newWire->removeLastPoint();
            // Move the new last point to where the previous last point was
            _newWire->move_point_to(_newWire->pointsAbsolute().count() - 1, mousePos);
        }
    }
}

/**
 * Removes the wires and wire nets that are not connected to a node
 */
void Scene::removeUnconnectedWires()
{
    QList<std::shared_ptr<Wire>> wiresToRemove;

    for (const auto& wire : m_wire_manager->wires()) {
        // If it has wires attached to it, go to the next wire
        if (wire->connected_wires().count() > 0) {
            continue;
        }

        bool isConnected = false;

        // Check if it is connected to a wire
        for (const auto& otherWire : m_wire_manager->wires()) {
            if (otherWire->connected_wires().contains(wire.get())) {
                isConnected = true;
                break;
            }
        }

        // If it's connected to a wire, go to the next wire
        if (isConnected) {
            continue;
        }

        // Find out if it's attached to a node
        for (const auto& connector : connectors()) {
            if (m_wire_manager->attached_wire(connector.get()) == wire.get()) {
                isConnected = true;
                break;
            }
        }

        // If it's connected to a connector, go to the next wire
        if (isConnected) {
            continue;
        }

        // The wire has to be removed, add it to the list
        if (auto wireItem = std::dynamic_pointer_cast<Wire>(wire)) {
            wiresToRemove << wireItem;
        }
    }

    // Remove the wires that have to be removed
    for (const auto& wire : wiresToRemove) {
        _undoStack->push(new CommandItemRemove(this, wire));
    }
}

bool Scene::addWire(const std::shared_ptr<Wire>& wire)
{
    if (!m_wire_manager->add_wire(wire)) {
        return false;
    }

    // Add wire to scene
    // Wires created by mouse interactions are already added to the scene in the Scene::mouseXxxEvent() calls. Prevent
    // adding an already added item to the scene
    if (wire->scene() != this) {
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

    // Disconnect from connectors
    for (const auto& connector: connectors()) {
        if (m_wire_manager->attached_wire(connector.get()) == wire.get()) {
            m_wire_manager->detach_wire(connector.get());
        }
    }

    return m_wire_manager->remove_wire(wire);
}


