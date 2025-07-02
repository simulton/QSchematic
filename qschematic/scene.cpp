#include <algorithm>

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QUndoStack>
#include <QMimeData>
#include <QtMath>
#include <QTimer>

#include "scene.hpp"
#include "background.hpp"
#include "commands/item_move.hpp"
#include "commands/item_add.hpp"
#include "commands/item_remove.hpp"
#include "items/itemfactory.hpp"
#include "items/item.hpp"
#include "items/itemmimedata.hpp"
#include "items/node.hpp"
#include "items/label.hpp"
#include "items/widget.hpp"
#include "utils/itemscontainerutils.hpp"

using namespace QSchematic;

Scene::Scene(QObject* parent) :
    QGraphicsScene(parent)
{
    // NOTE: still needed, BSP-indexer still crashes on a scene load when
    // the scene is already populated
    setItemIndexMethod(ItemIndexMethod::NoIndex);

    // Wire system
    m_wire_manager = std::make_shared<wire_system::manager>();
    m_wire_manager->set_net_factory([this] { return std::make_shared<Items::WireNet>(); });
    connect(m_wire_manager.get(), &wire_system::manager::wire_point_moved, this, &Scene::wirePointMoved);

    // Undo stack
    _undoStack = new QUndoStack(this);
    connect(_undoStack, &QUndoStack::cleanChanged, [this](bool isClean) {
        Q_EMIT isDirtyChanged(!isClean);
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

    // Background
    setupBackground();
    connect(this, &QGraphicsScene::sceneRectChanged, [this](const QRectF rect){
        if (_background) {
            // Note: We adjust the scene rect (make it smaller) before we set it as the background rect because the scene automatically resizes
            //       to accommodate all items. If we were not ot do this, the scene would resize (get larger) after setting the background rect
            //       triggering this slot again and we'd end up in an infinite loop.
            _background->setRect(rect.adjusted(1, 1, -1, -1));
        }
    });
}

Scene::~Scene()
{
    clear();
}

gpds::container
Scene::to_container() const
{
    // Root
    gpds::container c;
    c.add_attribute("version", serdes_version);

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
    c.add_value("scene", scene);

    // Items
    for (const auto& item : items()) {
        // Sanity check
        if (!item) [[unlikely]]
            continue;

        // Ignore wire items as they will be added by the nets below
        if (auto wire = std::dynamic_pointer_cast<Items::Wire>(item); wire)
            continue;

        c.add_value("item", item->to_container());
    }

    // Nets
    for (const auto& net : m_wire_manager->nets()) {
        // Make sure it's a WireNet
        auto wire_net = std::dynamic_pointer_cast<Items::WireNet>(net);
        if (!wire_net)
            continue;

        c.add_value("net", wire_net->to_container());
    }

    return c;
}

void
Scene::from_container(const gpds::container& container)
{
    // Check the version
    const std::size_t version = container.get_attribute<std::size_t>("version").value_or(-1);
    if (version != serdes_version)
        return;

    // Scene
    if (const gpds::container* sceneContainer = container.get_value<gpds::container*>("scene").value_or(nullptr); sceneContainer) {
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

    // Items
    for (const auto& itemContainer : container.get_values<gpds::container*>("item")) {
        if (!itemContainer)
            continue;

        auto item = Items::Factory::instance().from_container(*itemContainer);
        if (!item)
            continue;
        item->from_container(*itemContainer);
        addItem(item);
    }

    // Nets
    for (const gpds::container* netContainer : container.get_values<gpds::container*>("net")) {
        if (!netContainer)
            continue;

        auto net = std::make_shared<Items::WireNet>();
        net->setScene(this);
        net->set_manager(wire_manager().get());
        net->from_container( *netContainer );

        m_wire_manager->add_net(net);
    }

    // Attach the wires to the nodes
    generateConnections();

    // Find junctions
    m_wire_manager->generate_junctions();

    // Clear the undo history
    _undoStack->clear();
}

void
Scene::setSettings(const Settings& settings)
{
    // Update background
    if (_background)
        _background->setSettings(settings);

    // Update settings of all items
    for (auto& item : items())
        item->setSettings(settings);

    // Update settings of the wire manager
    m_wire_manager->set_settings(settings);

    // Store new settings
    _settings = settings;

    // Redraw
    update();
}

void
Scene::setWireFactory(const std::function<std::shared_ptr<Items::Wire>()>& factory)
{
    _wireFactory = factory;
}

void
Scene::setMode(int mode)
{
    // Don't do anything unnecessary
    if (mode == _mode)
        return;

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
    Q_EMIT modeChanged(_mode);
}

int
Scene::mode() const
{
    return _mode;
}

void
Scene::toggleWirePosture()
{
    _invertWirePosture = !_invertWirePosture;
}

bool
Scene::isDirty() const
{
    if (_undoStack)
        return !_undoStack->isClean();

    return false;
}

void
Scene::clearIsDirty()
{
    if (_undoStack)
        _undoStack->setClean();
}

void
Scene::clear()
{
    // Ensure no lingering lifespans kept in map-keys, selections or undocommands
    _initialItemPositions.clear();
    clearSelection();
    clearFocus();
    _undoStack->clear();

    // Remove from scene
    // Do not use QGraphicsScene::clear() as that would also delete the items. However,
    // we still need them as we manage them via smart pointers (eg. in commands)
    while (!_items.isEmpty())
        removeItem(_items.first());

    // Nets
    m_wire_manager->clear();

    // Now that all the top-level items are safeguarded we can call the underlying scene's clear()
    QGraphicsScene::clear();

    // No longer dirty
    clearIsDirty();

    // Setup the background again
    setupBackground();
}

bool
Scene::addItem(const std::shared_ptr<Items::Item>& item)
{
    // Sanity check
    if (!item)
        return false;

    // Setup item
    setupNewItem(*(item.get()));

    // Add to scene
    QGraphicsScene::addItem(item.get());

    // Store the shared pointer to keep the item alive for the QGraphicsScene
    _items << item;

    // Let the world know
    Q_EMIT itemAdded(item);
    Q_EMIT netlistChanged();

    return true;
}

bool
Scene::removeItem(const std::shared_ptr<Items::Item> item)
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
    Q_EMIT itemRemoved(item);
    Q_EMIT netlistChanged();

    // NOTE: In order to keep items alive through this entire event loop round,
    // otherwise crashes because Qt messes with items even after they're removed
    // ToDo: Fix this
    _keep_alive_an_event_loop << item;

    return true;
}

bool
Scene::isBackground(const QGraphicsItem* item) const
{
    // Note: Using dynamic_cast instead of qgraphicsitem_cast because we are not registering the background as a custom
    //       item.

    return dynamic_cast<const Background*>(item) == _background;
}

QList<std::shared_ptr<Items::Item>>
Scene::items() const
{
    return _items;
}

QList<std::shared_ptr<Items::Item>>
Scene::itemsAt(const QPointF &scenePos, Qt::SortOrder order) const
{
    return ItemUtils::mapItemListToSharedPtrList<QList>(QGraphicsScene::items(scenePos, Qt::IntersectsItemShape, order));
}

QList<std::shared_ptr<Items::Item>>
Scene::items(int itemType) const
{
    QList<std::shared_ptr<Items::Item>> items;

    for (auto& item : _items) {
        if (item->type() != itemType)
            continue;

        items << item;
    }

    return items;
}

std::vector<std::shared_ptr<Items::Item>>
Scene::selectedItems() const
{
    return ItemUtils::mapItemListToSharedPtrList<std::vector>(QGraphicsScene::selectedItems());
}

/**
 * Returns only the selected items that are not part of another item.
 * \remark The top-level items are those that were added to the scene by calling Scene::addItem.
 * Items that should not be top-level items need to be added by using QGraphicsScene::addItem
 * \returns List of selected top-level items
 */
std::vector<std::shared_ptr<Items::Item>>
Scene::selectedTopLevelItems() const
{
    const auto& rawItems = QGraphicsScene::selectedItems();
    std::vector<std::shared_ptr<Items::Item>> items;

    for (auto& item : _items) {
        if (rawItems.contains(item.get()))
            items.push_back(item);
    }

    return items;
}

QList<std::shared_ptr<Items::Node>>
Scene::nodes() const
{
    QList<std::shared_ptr<Items::Node>> nodes;

    for (auto& item : _items) {
        auto node = std::dynamic_pointer_cast<Items::Node>(item);
        if (!node)
            continue;

        nodes << node;
    }

    return nodes;
}

std::shared_ptr<Items::Node>
Scene::nodeFromConnector(const QSchematic::Items::Connector& connector) const
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

void
Scene::undo()
{
    _undoStack->undo();
}

void
Scene::redo()
{
    _undoStack->redo();
}

QUndoStack*
Scene::undoStack() const
{
    return _undoStack;
}

std::shared_ptr<wire_system::manager>
Scene::wire_manager() const
{
    return m_wire_manager;
}

void
Scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
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
            _movingNodes = false;
            if (QGraphicsItem* item = itemAt(event->scenePos(), QTransform()); item) {
                Items::Node* node = dynamic_cast<Items::Node*>(item);
                if (node && node->mode() == Items::Node::Mode::None)
                    _movingNodes = true;
                else
                    _movingNodes = false;

                // Prevent the scene from detecting changes in the wires origin
                // when the bounding rect is resized by a wire_point that moved
                if (Items::Wire* wire = dynamic_cast<Items::Wire*>(item); wire) {
                    if (wire->movingWirePoint())
                        _movingNodes = false;
                    else
                        _movingNodes = true;
                }

                // Label
                Items::Label* label = dynamic_cast<Items::Label*>(item);
                if (label && selectedTopLevelItems().size() > 0)
                    _movingNodes = true;
            }

            // Store the initial position of all the selected items
            _initialItemPositions.clear();
            for (auto& item: selectedTopLevelItems()) {
                if (item)
                    _initialItemPositions.insert(item, item->pos());
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
                    _undoStack->push(new Commands::ItemAdd(this, _newWire));
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
                            m_wire_manager->attach_wire_to_connector(_newWire.get(), _newWire->pointsAbsolute().indexOf(snappedPos), connector.get());
                            wireAttached = true;
                            break;
                        }
                    }
                }

                // Attach point to wire if needed
                for (const auto& wire: m_wire_manager->wires()) {
                    // Skip current wire
                    if (wire == _newWire)
                        continue;

                    if (wire->point_is_on_wire(_newWire->pointsAbsolute().last())) {
                        m_wire_manager->connect_wire(wire.get(), _newWire.get(), _newWire->pointsAbsolute().count() - 1);
                        wireAttached = true;
                        break;
                    }
                }

                // Check if both ends of the wire are connected to something
                if (wireAttached && _newWire->pointsAbsolute().count() > 1)
                    finishCurrentWire();
            }

            break;
        }
    }

    _lastMousePos = event->scenePos();
}

void
Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    switch (_mode) {
        case NormalMode:
        {
            QGraphicsScene::mouseReleaseEvent(event);

            for (const auto& net : m_wire_manager->nets()) {
                // Make sure it's a WireNet
                auto wire_net = std::dynamic_pointer_cast<Items::WireNet>(net);
                if (!wire_net)
                    continue;

                wire_net->updateLabelPos(true);
            }

            // Reset the position for every selected item and
            // apply the translation through the undostack
            if (_movingNodes) {
                // Get list of items that need to be moved
                // Note: We want/need to ensure that all `Wire` items are first in the list!
                QVector<std::shared_ptr<Items::Item>> itemsToMove;
                for (const auto& item : selectedTopLevelItems()) {
                    // Ignore items not marked as movable
                    if (!item->isMovable())
                        continue;

                    itemsToMove << item;
                }
                std::partition(
                    std::begin(itemsToMove),
                    std::end(itemsToMove),
                    [](const std::shared_ptr<Items::Item>& item) -> bool {
                        return std::dynamic_pointer_cast<Items::Wire>(item) != nullptr;
                    }
                );

                QVector2D moveBy;
                for (const auto& item : itemsToMove) {
                    // Move the item if it is movable and it was previously registered by the mousePressEvent
                    moveBy = QVector2D(item->pos() - _initialItemPositions.value(item));

                    // Move the item to its initial position
                    item->setPos(_initialItemPositions.value(item));
                }

                // Apply the translation
                if (!moveBy.isNull())
                    _undoStack->push(new Commands::ItemMove(itemsToMove, moveBy));

                for (const auto& item : itemsToMove) {
                    if (const Items::Node* node = dynamic_cast<const Items::Node*>(item.get()); node)
                        updateNodeConnections(node);
                }
            }
            break;
        }

        case WireMode:
        {
            // Right mouse button: Abort wire mode
            if (event->button() == Qt::RightButton) {

                // Change the mode back to NormalMode if nothing below cursor
                if (QGraphicsScene::items(event->scenePos()).isEmpty())
                    setMode(NormalMode);

                // Show the context menu stuff
                QGraphicsScene::mouseReleaseEvent(event);
            }

            break;
        }
    }

    _lastMousePos = event->lastScenePos();
}

void
Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
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
                    // Get list of items that need to be moved
                    // Note: We want/need to ensure that all `Wire` items are first in the list!
                    QVector<std::shared_ptr<Items::Item>> itemsToMove;
                    for (const auto& item : selectedTopLevelItems()) {
                        // Ignore items not marked as movable
                        if (!item->isMovable())
                            continue;

                        itemsToMove << item;
                    }
                    std::partition(
                        std::begin(itemsToMove),
                        std::end(itemsToMove),
                        [](const std::shared_ptr<Items::Item>& item) -> bool {
                            return std::dynamic_pointer_cast<Items::Wire>(item) != nullptr;
                        }
                    );

                    for (const auto& item : itemsToMove) {
                        // Calculate by how much the item was moved
                        QVector2D moveBy{ _initialItemPositions.value(item) + newMousePos - _initialCursorPosition - item->pos() };
                        
                        // Apply the custom scene snapping
                        moveBy = itemsMoveSnap(item, moveBy);
                        item->setPos(item->pos() + moveBy.toPointF());
                    }

                    // Simplify all the wires
                    for (auto& wire : m_wire_manager->wires())
                        wire->simplify();
                }
                else
                    QGraphicsScene::mouseMoveEvent(event);
            }
            else
                QGraphicsScene::mouseMoveEvent(event);

            // Highlight the item under the cursor
            Items::Item* item = dynamic_cast<Items::Item*>(itemAt(newMousePos, QTransform()));
            if (item && item->highlightEnabled()) {
                // Skip if the item is already highlighted
                if (item == _highlightedItem.get())
                    break;

                // Disable the highlighting on the previous item
                if (_highlightedItem) {
                    _highlightedItem->setHighlighted(false);
                    itemHoverLeave(_highlightedItem->shared_from_this());
                    _highlightedItem->update();
                    Q_EMIT _highlightedItem->highlightChanged(*_highlightedItem, false);
                    _highlightedItem = { };
                }

                // Highlight the item
                item->setHighlighted(true);
                itemHoverEnter(item->shared_from_this());
                item->update();
                Q_EMIT item->highlightChanged(*item, true);
                _highlightedItem = item->shared_from_this();
            }

            // No item selected
            else if (_highlightedItem) {
                _highlightedItem->setHighlighted(false);
                itemHoverLeave(_highlightedItem->shared_from_this());
                _highlightedItem->update();
                Q_EMIT _highlightedItem->highlightChanged(*_highlightedItem, false);
                _highlightedItem = nullptr;
            }

            break;
        }

        case WireMode:
        {
            // Make sure that there's a wire
            if (!_newWire)
                break;

            // Transform mouse coordinates to grid positions (snapped to nearest grid point)
            const QPointF& snappedPos = _settings.snapToGrid(event->scenePos());

            // Add a new wire segment. Only allow straight angles (if supposed to)
            if (_settings.routeStraightAngles) {
                if (_newWireSegment) {
                    // Remove the last point if there was a previous segment
                    if (_newWire->pointsRelative().count() > 1)
                        _newWire->removeLastPoint();

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
                }
                else {
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
            }
            else {
                // Don't care about angles and stuff. Fuck geometry, right?
                if (_newWire->pointsAbsolute().count() > 1)
                    _newWire->move_point_to(_newWire->pointsAbsolute().count() - 1, snappedPos);
                else
                    _newWire->append_point(snappedPos);
            }

            break;
        }

    }

    // Save the last mouse position
    _lastMousePos = newMousePos;
}

void
Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
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
                    if (wire == _newWire)
                        continue;

                    if (wire->point_is_on_wire(_newWire->pointsAbsolute().last()))
                        m_wire_manager->connect_wire(wire.get(), _newWire.get(), _newWire->pointsAbsolute().count() - 1);
                }

                // Finish the current wire
                finishCurrentWire();

                return;
            }

            return;
        }
    }
}

void
Scene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    // Create a list of mime formats we can handle
    QStringList mimeFormatsWeCanHandle {
        Items::MIME_TYPE_NODE,
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

void
Scene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->acceptProposedAction();
}

void
Scene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->acceptProposedAction();
}

void
Scene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    event->accept();

    // Check the mime data
    const QMimeData* mimeData = event->mimeData();
    if (!mimeData)
        return;

    // Nodes
    if (mimeData->hasFormat(Items::MIME_TYPE_NODE)) {
        // Get the ItemMimeData
        const Items::MimeData* mimeData = qobject_cast<const Items::MimeData*>(event->mimeData());
        if (!mimeData)
            return;

        // Get the Item
        auto item = mimeData->item();
        if (!item)
            return;

        // Add to the scene
        item->setPos(event->scenePos());
        _undoStack->push(new Commands::ItemAdd(this, std::move(item)));
    }
}

std::unique_ptr<Background>
Scene::makeBackground() const
{
    return std::make_unique<Background>();
}

QVector2D
Scene::itemsMoveSnap(const std::shared_ptr<Items::Item>& items, const QVector2D& moveBy) const
{
    Q_UNUSED(items);

    return moveBy;
}

void
Scene::setupBackground()
{
    auto bg = makeBackground();
    if (!bg)
        return;

    // Configure
    bg->setRect(sceneRect());
    bg->setZValue(z_value_background);
    bg->setSettings(_settings);

    // Bookkeeping
    _background = bg.release();

    // Add to scene
    QGraphicsScene::addItem(_background);
}

void
Scene::updateNodeConnections(const Items::Node* node)
{
    // Check if a connector lays on a wirepoint
    for (auto& connector : node->connectors()) {
        // Skip hidden connectors
        if (!connector->isVisible())
            continue;

        // Get the connection record (if any)
        auto cr = m_wire_manager->attached_wire(connector.get());

        // If the connector already has a wire attached, skip
        if (cr && cr->wire != nullptr)
            continue;

        // Find if there is a point to connect to
        // Note: We only consider the first and the last point of a wire as new legal connections
        for (const auto& wire : m_wire_manager->wires()) {
            int index = -1;

            if (wire->points().first().toPoint() == connector->scenePos().toPoint())
                index = 0;
            else if (wire->points().last().toPoint() == connector->scenePos().toPoint())
                index = wire->points().count() - 1;

            if (index != -1) {
                // Ignore if it's a junction
                if (wire->points().at(index).is_junction())
                    continue;

                // Check if it isn't already connected to another connector
                bool alreadyConnected = false;
                for (const auto& otherConnector : connectors()) {
                    if (otherConnector == connector)
                        continue;

                    // Get the connection record of the other connector
                    const auto crOther = m_wire_manager->attached_wire(otherConnector.get());
                    if (!crOther)
                        continue;

                    if (cr) {
                        if (cr->wire == wire.get() && crOther->point_index == index) {
                            alreadyConnected = true;
                            break;
                        }
                    }
                }

                // If it's not already connected, connect it
                if (!alreadyConnected) {
                    m_wire_manager->attach_wire_to_connector(wire.get(), index, connector.get());
                    Q_EMIT netlistChanged();
                }
            }
        }
    }
}

void
Scene::wirePointMoved(wire& rawWire, int index)
{
    // Detach from connector
    for (const auto& node: nodes()) {
        for (const auto& connector: node->connectors()) {

            // Get the connection record
            const auto cr = m_wire_manager->attached_wire(connector.get());
            if (!cr)
                continue;

            if (!cr->wire)
                continue;

            if (cr->wire != &rawWire)
                continue;

            if (cr->point_index == index) {
                if (connector->scenePos().toPoint() != rawWire.points().at(index).toPoint())
                    m_wire_manager->detach_wire(connector.get());
            }
        }
    }

    // Attach to connector
    point point = rawWire.points().at(index);
    for (const auto& node: nodes()) {
        for (const auto& connector: node->connectors()) {
            if (connector->scenePos().toPoint() == point.toPoint())
                m_wire_manager->attach_wire_to_connector(&rawWire, index, connector.get());
        }
    }

    Q_EMIT netlistChanged();
}

void
Scene::setupNewItem(Items::Item& item)
{
    // Set settings
    item.setSettings(_settings);
}

void
Scene::generateConnections()
{
    for (const auto& connector : connectors()) {
        std::shared_ptr<wire> wire = m_wire_manager->wire_with_extremity_at(connector->scenePos());
        if (wire)
            m_wire_manager->attach_wire_to_connector(wire.get(), connector.get());
    }

    Q_EMIT netlistChanged();
}

/**
 * Finishes the current wire if there is one
 */
void
Scene::finishCurrentWire()
{
    if (!_newWire)
        return;

    // Finish the current wire
    _newWire->setAcceptHoverEvents(true);
    _newWire->setFlag(QGraphicsItem::ItemIsSelectable, true);
    _newWire->simplify();
//    _newWire->updatePosition();
    _newWire.reset();

    Q_EMIT netlistChanged();
}

std::shared_ptr<Items::Wire>
Scene::make_wire() const
{
    if (_wireFactory)
        return _wireFactory();
    else
        return std::make_shared<Items::Wire>();
}

QList<QPointF>
Scene::connectionPoints() const
{
    QList<QPointF> list;

    for (const auto& node : nodes())
        list << node->connectionPointsAbsolute();

    return list;
}

QList<std::shared_ptr<Items::Connector>>
Scene::connectors() const
{
    QList<std::shared_ptr<Items::Connector>> list;

    for (const auto& node : nodes())
        list << node->connectors();

    return list;
}

void
Scene::itemHoverEnter(const std::shared_ptr<const Items::Item>& item)
{
    Q_EMIT itemHighlighted(item);

    // Start popup timer
    _popupTimer->start(_settings.popupDelay);
}

void
Scene::itemHoverLeave([[maybe_unused]] const std::shared_ptr<const Items::Item>& item)
{
    // ToDo: clear _highlightedItem ?

    Q_EMIT itemHighlighted(nullptr);

    // Stop popup timer
    _popupTimer->stop();
    _popup = { };
}

/**
 * Removes the last point(s) of the new wire. After execution, the wire should
 * be in the same state it was before the last point had been added.
 */
void
Scene::removeLastWirePoint()
{
    if (!_newWire)
        return;

    // If we're supposed to preserve right angles, two points have to be removed
    if (_settings.routeStraightAngles) {
        // Do nothing if there are not at least 4 points
        if (_newWire->pointsAbsolute().count() > 3) {
            // Keep the position of the last point
            QPointF mousePos = _newWire->pointsAbsolute().last();

            // Remove both points
            _newWire->removeLastPoint();
            _newWire->removeLastPoint();

            // Move the new last point to where the previous last point was
            _newWire->move_point_by(_newWire->pointsAbsolute().count() - 1, QVector2D(mousePos - _newWire->pointsAbsolute().last()));
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

    Q_EMIT netlistChanged();
}

/**
 * Removes the wires and wire nets that are not connected to a node
 */
void
Scene::removeUnconnectedWires()
{
    QList<std::shared_ptr<Items::Wire>> wiresToRemove;

    for (const auto& wire : m_wire_manager->wires()) {
        // If it has wires attached to it, go to the next wire
        if (wire->connected_wires().count() > 0)
            continue;

        bool isConnected = false;

        // Check if it is connected to a wire
        for (const auto& otherWire : m_wire_manager->wires()) {
            if (otherWire->connected_wires().contains(wire.get())) {
                isConnected = true;
                break;
            }
        }

        // If it's connected to a wire, go to the next wire
        if (isConnected)
            continue;

        // Find out if it's attached to a node
        for (const auto& connector : connectors()) {
            if (m_wire_manager->is_wire_attached_to(wire.get(), connector.get())) {
                isConnected = true;
                break;
            }
        }

        // If it's connected to a connector, go to the next wire
        if (isConnected)
            continue;

        // The wire has to be removed, add it to the list
        if (auto wireItem = std::dynamic_pointer_cast<Items::Wire>(wire))
            wiresToRemove << wireItem;
    }

    // Remove the wires that have to be removed
    for (const auto& wire : wiresToRemove)
        _undoStack->push(new Commands::ItemRemove(this, wire));

    Q_EMIT netlistChanged();
}

bool
Scene::addWire(const std::shared_ptr<Items::Wire>& wire)
{
    if (!m_wire_manager->add_wire(wire))
        return false;

    // Add wire to scene
    // Wires created by mouse interactions are already added to the scene in the Scene::mouseXxxEvent() calls. Prevent
    // adding an already added item to the scene
    if (wire->scene() != this) {
        if (!addItem(wire))
            return false;
    }

    Q_EMIT netlistChanged();

    return true;
}

void
Scene::removeWire(const std::shared_ptr<Items::Wire>& wire)
{
    // Remove the wire from the scene
    removeItem(wire);

    // Remove the wire from the wire system
    m_wire_manager->remove_wire(wire);

    Q_EMIT netlistChanged();
}
