#include <algorithm>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QUndoStack>
#include <QMimeData>
#include <QtMath>
#include <QTimer>

#include "scene.h"
#include "commands/commanditemmove.h"
#include "commands/commanditemadd.h"
#include "items/itemfactory.h"
#include "items/item.h"
#include "items/itemmimedata.h"
#include "items/node.h"
#include "items/wire.h"
#include "items/wirenet.h"
#include "items/label.h"
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
    for (const auto& net : nets()) {
        netsList.add_value("net", net->to_container());
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
                qCritical("Scene::from_container(): Couldn't restore node. Skipping.");
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
            net->from_container( *netContainer );

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

    // Find junctions
    for (const auto& wire: wires()) {
        for (auto& otherWire: wires()) {
            if (wire == otherWire) {
                continue;
            }
            if (wire->pointIsOnWire(otherWire->wirePointsAbsolute().first().toPointF())) {
                connectWire(wire, otherWire);
                otherWire->setPointIsJunction(0, true);
            }
            if (wire->pointIsOnWire(otherWire->wirePointsAbsolute().last().toPointF())) {
                connectWire(wire, otherWire);
                otherWire->setPointIsJunction(otherWire->wirePointsAbsolute().count()-1, true);
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
    _undoStack->clear();

    // Remove from scene
    // Do not use QGraphicsScene::clear() as that would also delete the items. However,
    // we still need them as we manage them via smart pointers (eg. in commands)
    while (!_items.isEmpty()) {
        removeItem(_items.first());
    }

    // Nets
    _nets.clear();

    // Now that all the top-level items are safeguarded we can call the underlying scene's clear()
#warning "Address this issue..."
    //QGraphicsScene::clear();
    const int& itemsCount = QGraphicsScene::items().count();
    if (itemsCount > 0)
        qWarning("Scene::clear(): There are still %d items left in the scene. This shouldn't happen.", itemsCount);

    clearIsDirty();

    // Update
    update(); // Note, should not be needed, and not recommended according to input, but avoid adding yet a permutation to the investigation
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
    if (!item) {
        return false;
    }

    auto itemBoundsToUpdate = item->mapRectToScene(item->boundingRect());

    // NOTE: Sometimes ghosts remain (not drawn away) when they're active in some way at remove time, found below from looking at Qt-source code...
    item->clearFocus();
    item->setFocusProxy(nullptr);

    // Remove from scene (if necessary)
    if (item->QGraphicsItem::scene()) {
       QGraphicsScene::removeItem(item.get());
    }

    // Remove shared pointer from local list to reduce instance count
    _items.removeAll(item);

    update(itemBoundsToUpdate);

    // Let the world know
    emit itemRemoved(item);

    // NOTE: In order to keep items alive through this entire event loop round,
    // otherwise crashes because Qt messes with items even after they're removed
    // (though, again, seems limited to when BSP-index on [as "litmus test"])
    if (_keep_alive_an_event_loop.isEmpty()) {
        QTimer::singleShot(0, [this]{
            _keep_alive_an_event_loop.clear();
        });
    }
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

bool Scene::addWire(const std::shared_ptr<Wire>& wire)
{
    // Sanity check
    if (!wire) {
        return false;
    }

    // No point of the new wire lies on an existing line segment - create a new wire net
    auto newNet = std::make_shared<WireNet>();
    newNet->addWire(wire);
    addWireNet(newNet);

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

bool Scene::removeWire(const std::shared_ptr<Wire> wire)
{
    // Remove the wire from the scene
    removeItem(wire);

    // Disconnect from connectors
    for (const auto& connector: connectors()) {
        if (connector->attachedWire() == wire.get()) {
            connector->detachWire();
        }
    }

    // Disconnect from connected wires
    for (const auto& otherWire: wiresConnectedTo(wire)) {
        if (otherWire != wire) {
            disconnectWire(otherWire, wire);
            // Update the junction on the other wire
            for (int index = 0; index < otherWire->pointsAbsolute().count(); index++) {
                const auto point = otherWire->wirePointsAbsolute().at(index);
                if (not point.isJunction()) {
                    continue;
                }
                if (wire->pointIsOnWire(point.toPointF())) {
                    otherWire->setPointIsJunction(index, false);
                }
            }
        }
    }

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
        removeWireNet(net);
    }

    return true;
}

QList<std::shared_ptr<Wire>> Scene::wires() const
{
    QList<std::shared_ptr<Wire>> list;

    for (const auto& wireNet : _nets) {
        for (const auto& wire : wireNet->wires()) {
            list.append(wire);
        }
    }

    return list;
}

QList<std::shared_ptr<WireNet>> Scene::nets() const
{
    return _nets;
}

QList<std::shared_ptr<WireNet>> Scene::nets(const std::shared_ptr<WireNet> wireNet) const
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

std::shared_ptr<WireNet> Scene::net(const std::shared_ptr<Wire> wire) const
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
    Q_UNUSED(rawWire)
    Q_UNUSED(point)
}

void Scene::wirePointMovedByUser(Wire& rawWire, int index)
{
    WirePoint point = rawWire.wirePointsRelative().at(index);
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

    // Detach wires
    if (index == 0 or index == rawWire.pointsAbsolute().count()-1){
        if (point.isJunction()) {
            for (const auto& wire: wires()) {
                // Skip current wire
                if (wire.get() == &rawWire) {
                    continue;
                }
                // If is connected
                if (wire->connectedWires().contains(&rawWire)) {
                    bool shouldDisconnect = true;
                    // Keep the wires connected if there is another junction
                    for (const auto& jIndex : rawWire.junctions()) {
                        const auto& junction = rawWire.wirePointsAbsolute().at(jIndex);
                        // Ignore the point that moved
                        if (jIndex == index) {
                            continue;
                        }
                        // If the point is on the line stay connected
                        if (wire->pointIsOnWire(junction.toPointF())) {
                            shouldDisconnect = false;
                            break;
                        }
                    }
                    if (shouldDisconnect) {
                        auto rawWirePtr = std::static_pointer_cast<Wire>(rawWire.sharedPtr());
                        disconnectWire(wire, rawWirePtr);
                    }
                    rawWire.setPointIsJunction(index, false);
                }
            }
        }
    }

    // Attach point to wire if needed
    if (index == 0 or index == rawWire.wirePointsAbsolute().count()-1) {
        for (const auto& wire: wires()) {
            // Skip current wire
            if (wire.get() == &rawWire) {
                continue;
            }
            if (wire->pointIsOnWire(rawWire.wirePointsAbsolute().at(index).toPointF())) {
                if (not rawWire.connectedWires().contains(wire.get())) {
                    rawWire.setPointIsJunction(index, true);
                    auto rawWirePtr = std::static_pointer_cast<Wire>(rawWire.sharedPtr());
                    connectWire(wire, rawWirePtr);
                }
            }
        }
    }
}

/**
 * Disconnects the a wire from another and takes care of updating the wirenets.
 * \param wire The wire that the other is attached to
 * \param otherWire The wire that is being disconnected
 */
void Scene::disconnectWire(const std::shared_ptr<Wire>& wire, const std::shared_ptr<Wire>& otherWire)
{
    wire->disconnectWire(otherWire.get());
    auto net = otherWire->net();
    // Create a list of wires that will stay in the old net
    QVector<std::shared_ptr<Wire>> oldWires = wiresConnectedTo(wire);
    // If there are wires that are not in the list create a new net
    if (net->wires().count() != oldWires.count()) {
        // Create new net and add the wire
        auto newNet = std::make_shared<WireNet>();
        addWireNet(newNet);
        for (auto wireToMove: net->wires()) {
            if (oldWires.contains(wireToMove)) {
                continue;
            }
            newNet->addWire(wireToMove);
            net->removeWire(wireToMove);
        }
    }
}

/**
 * Generates a list of all the wires connected to a certain wire including the
 * wire itself.
 */
QVector<std::shared_ptr<Wire>> Scene::wiresConnectedTo(const std::shared_ptr<Wire>& wire) const
{
    QVector<std::shared_ptr<Wire>> connectedWires;

    // Add the wire itself to the list
    connectedWires.push_back(wire);

    QVector<std::shared_ptr<Wire>> newList;
    do {
        newList.clear();
        // Go through all the wires in the net
        for (const auto& otherWire: wire->net()->wires()) {
            // Ignore if the wire is already in the list
            if (connectedWires.contains(otherWire)) {
                continue;
            }

            // If they are connected to one of the wire in the list add them to the new list
            for (const auto& wire2 : connectedWires) {
                if (wire2->connectedWires().contains(otherWire.get())) {
                    newList << otherWire;
                    break;
                }
                if (otherWire->connectedWires().contains(wire2.get())) {
                    newList << otherWire;
                    break;
                }
            }
        }

        connectedWires << newList;
    } while (not newList.isEmpty());

    return connectedWires;
}

/**
 * Connect a wire to another wire while taking care of merging the nets.
 * @param wire The wire to connect to
 * @param rawWire The wire to connect
 */
void Scene::connectWire(const std::shared_ptr<Wire>& wire, std::shared_ptr<Wire>& rawWire)
{
    if (not wire->connectWire(rawWire.get())) {
        return;
    }
    std::shared_ptr<WireNet> net = netFromWire(wire);
    std::shared_ptr<WireNet> otherNet = netFromWire(rawWire);
    if (mergeNets(net, otherNet)) {
        removeWireNet(otherNet);
    }
}

/**
 * Recusivelly move a wire and all the wires attached to it to a wirenet.
 */
void Scene::moveWireToNet(std::shared_ptr<Wire>& rawWire, std::shared_ptr<WireNet>& newNet) const
{
    std::shared_ptr<WireNet> net = netFromWire(rawWire);
    newNet->addWire(rawWire);
    net->removeWire(rawWire);
    for (const auto wire: rawWire->connectedWires()) {
        auto wirePtr = std::static_pointer_cast<Wire>(wire->sharedPtr());
        moveWireToNet(wirePtr, newNet);
    }
}

/**
 * Merges two wirenets into one
 * \param net The net into which the other one will be merged
 * \param otherNet The net to merge into the other one
 * \return Whether the two nets where merged successfully or not
 */
bool Scene::mergeNets(std::shared_ptr<WireNet>& net, std::shared_ptr<WireNet>& otherNet)
{
    // Ignore if it's the same net
    if (net == otherNet) {
        return false;
    }
    for (auto& wire: otherNet->wires()) {
        net->addWire(wire);
        otherNet->removeWire(wire);
    }
    return true;
}

std::shared_ptr<WireNet> Scene::netFromWire(const std::shared_ptr<Wire>& wire) const
{
    for (auto& otherNet : _nets) {
        if (otherNet->contains(wire)) {
           return otherNet;
        }
    }
    return nullptr;
}

void Scene::addWireNet(const std::shared_ptr<WireNet> wireNet)
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

/**
 * Finishes the current wire if there is one
 */
void Scene::finishCurrentWire()
{
    if (not _newWire) {
        return;
    }
    // Finish the current wire
    _newWire->setAcceptHoverEvents(true);
    _newWire->setFlag(QGraphicsItem::ItemIsSelectable, true);
    _newWire->simplify();
    _newWire->updatePosition();
    _newWire.reset();
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

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
            if (node and node->mode() == Node::None) {
                _movingNodes = true;
            } else {
                _movingNodes = false;
            }
            // Prevent the scene from detecting changes in the wires origin
            // when the bouding rect is resized by a WirePoint that moved
            Wire* wire = dynamic_cast<Wire*>(item);
            if (wire) {
                if (wire->movingWirePoint()) {
                    _movingNodes = false;
                } else {
                    _movingNodes = true;
                }
            }
            Label* label = dynamic_cast<Label*>(item);
            if (label and selectedTopLevelItems().size() > 0) {
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
                if (_wireFactory) {
                    _newWire = adopt_origin_instance(_wireFactory());
                } else {
                    _newWire = mk_sh<Wire>();
                }
                _undoStack->push(new CommandItemAdd(this, _newWire));
                _newWire->setPos(_settings.snapToGrid(event->scenePos()));
            }
            // Snap to grid
            const QPointF& snappedPos = _settings.snapToGrid(event->scenePos());
            _newWire->appendPoint(snappedPos);
            _newWireSegment = true;

            // Attach point to connector if needed
            bool wireAttached = false;
            for (const auto& node: nodes()) {
                for (const auto& connector: node->connectors()) {
                    if (QVector2D(connector->scenePos() - snappedPos).length() < 1) {
                        connector->attachWire(_newWire.get(), _newWire->pointsAbsolute().indexOf(snappedPos));
                        wireAttached = true;
                        break;
                    }
                }
            }

            // Attach point to wire if needed
            for (const auto& wire: wires()) {
                // Skip current wire
                if (wire == _newWire) {
                    continue;
                }
                if (wire->pointIsOnWire(_newWire->pointsAbsolute().last())) {
                    connectWire(wire, _newWire);
                    _newWire->setPointIsJunction(_newWire->pointsAbsolute().count() - 1, true);
                    wireAttached = true;
                    break;
                }
            }

            // Check if both ends of the wire are connected to something
            if (wireAttached and _newWire->pointsAbsolute().count() > 1) {
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

        for (const auto& net : nets()) {
            net->updateLabelPos(true);
        }
        // Reset the position for every selected item and
        // apply the translation through the undostack
        if (_movingNodes) {
            QVector<std::shared_ptr<Item>> wiresToMove;
            QVector<std::shared_ptr<Item>> itemsToMove;

            for (const auto& item : selectedTopLevelItems()) {
                if (item->isMovable() and _initialItemPositions.contains(item)) {
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
                if (not moveBy.isNull()) {
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

            for (auto& item : wires()) {
                Wire* wire = dynamic_cast<Wire*>(item.get());
                if (wire) {
                    wire->updatePosition();
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
        // If the connector already has a wire attached, skip
        if (connector->attachedWire() != nullptr) {
            continue;
        }
        // Find if there is a point to connect to
        for (const auto& wire : wires()) {
            int index = -1;
            if (wire->wirePointsAbsolute().first().toPoint() == connector->scenePos().toPoint()) {
                index = 0;
            } else if (wire->wirePointsAbsolute().last().toPoint() == connector->scenePos().toPoint()) {
                index = wire->wirePointsAbsolute().count() - 1;
            }
            if (index != -1) {
                // Ignore if it's a junction
                if (wire->wirePointsAbsolute().at(index).isJunction()){
                    continue;
                }
                // Check if it isn't already connected to another connector
                bool alreadyConnected = false;
                for (const auto& otherConnector : connectors()) {
                    if (otherConnector == connector) {
                        continue;
                    }
                    if (otherConnector->attachedWire() == wire.get() and
                        otherConnector->attachedWirepoint() == index) {
                        alreadyConnected = true;
                        break;
                    }
                }
                // If it's not already connected, connect it
                if (not alreadyConnected) {
                    connector->attachWire(wire.get(), index);
                }
            }
        }
    }
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
                for (auto& wire : wires()) {
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
        if (item) {
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
                WirePoint prevNode(_newWire->pointsAbsolute().at(_newWire->pointsAbsolute().count()-1));
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
                WirePoint p1(_newWire->pointsAbsolute().at(_newWire->pointsAbsolute().count()-3));
                QPointF p2(p1.x(), snappedPos.y());
                QPointF p3(snappedPos);
                if (_invertWirePosture) {
                    p2.setX(p3.x());
                    p2.setY(p1.y());
                }

                // Modify the actual wire
                _newWire->movePointTo(_newWire->pointsAbsolute().count()-2, p2);
                _newWire->movePointTo(_newWire->pointsAbsolute().count()-1, p3);
            }
        } else {
            // Don't care about angles and stuff. Fuck geometry, right?
            if (_newWire->pointsAbsolute().count() > 1) {
                _newWire->movePointTo(_newWire->pointsAbsolute().count()-1, snappedPos);
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
                if (connectionPoint == _newWire->pointsAbsolute().last()) {
                    wireIsFloating = false;
                    break;
                }
            }

            // Attach point to wire if needed
            for (const auto& wire: wires()) {
                // Skip current wire
                if (wire == _newWire) {
                    continue;
                }
                if (wire->pointIsOnWire(_newWire->pointsAbsolute().last())) {
                    connectWire(wire, _newWire);
                    _newWire->setPointIsJunction(_newWire->pointsAbsolute().count()-1, true);
                    wireIsFloating = false;
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

void Scene::removeWireNet(std::shared_ptr<WireNet> net)
{
    _nets.removeAll(net);
}

void Scene::itemHoverEnter(const std::shared_ptr<const Item>& item)
{
    emit itemHighlighted(item);
}

void Scene::itemHoverLeave([[maybe_unused]] const std::shared_ptr<const Item>& item)
{
    emit itemHighlighted(nullptr);
}
