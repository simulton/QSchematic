#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include "scene.h"
#include "settings.h"
#include "items/item.h"
#include "items/node.h"
#include "items/connector.h"
#include "items/wire.h"
#include "items/wirenet.h"

using namespace QSchematic;

Scene::Scene(QObject* parent) :
    QGraphicsScene(parent),
    _mode(NormalMode),
    _newWireSegment(false),
    _invertWirePosture(false)
{
}

void Scene::setSettings(const Settings& settings)
{
    // Update settings of all items
    for (Item* item : items()) {
        item->setSettings(settings);
    }

    // Store new settings
    _settings = settings;

    // Redraw
    update();
}

void Scene::setMode(Scene::Mode mode)
{
    // Check what the previous mode was
    switch (_mode) {

    // Discard current wire/bus
    case WireMode:
        _newWire.reset(nullptr);
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

bool Scene::addItem(Item* item)
{
    // Set settings
    item->setSettings(_settings);

    // Connections
    connect(item, &Item::showPopup, this, &Scene::showPopup);

    // Add to scene
    QGraphicsScene::addItem(item);

    return true;
}

QList<Item*> Scene::items() const
{
    QList<Item*> items;

    for (QGraphicsItem* i : QGraphicsScene::items()) {
        Item* item = dynamic_cast<Item*>(i);
        if (!item) {
            continue;
        }

        items << item;
    }

    return items;
}

QList<Item*> Scene::items(Item::ItemType itemType) const
{
    QList<Item*> items;

    for (QGraphicsItem* i : QGraphicsScene::items()) {
        Item* item = dynamic_cast<Item*>(i);
        if (!item or item->type() != itemType) {
            continue;
        }

        items << item;
    }

    return items;
}

QList<Node*> Scene::nodes() const
{
    QList<Node*> nodes;

    for (QGraphicsItem* i : QGraphicsScene::items()) {
        Node* node = dynamic_cast<Node*>(i);
        if (!node or node->type() != Item::NodeType) {
            continue;
        }

        nodes << node;
    }

    return nodes;
}

bool Scene::addWire(Wire* wire)
{
    if (!wire)
        return false;

    // Check if any point of the wire lies on any line segment of all existing line segments.
    // If yes, add to that net. Otherwise, create a new one
    for (WireNet* net : _nets) {
        for (const Line& line : net->lineSegments()) {
            for (const WirePoint& point : wire->points()) {
                if (line.containsPoint(point.toPoint(), 0)) {
                    net->addWire(*wire);
                    return true;
                }
            }
        }
    }

    // Check if any line segment of the wire lies on any point of all existing wires.
    // If yes, add to that net. Otherwise, create a new one
    for (WireNet* net : _nets) {
        for (const Wire* otherWire : net->wires()) {
            for (const WirePoint& otherPoint : otherWire->points()) {
                for (const Line& line : wire->lineSegments()) {
                    if (line.containsPoint(otherPoint.toPoint())) {
                        net->addWire(*wire);
                        return true;
                    }
                }
            }
        }
    }

    // No point of the new wire lies on an existing line segment - create a new wire net
    WireNet* newNet = new WireNet;
    addWireNet(newNet);
    newNet->addWire(*wire);

    return true;
}

bool Scene::removeWire(Wire& wire)
{
    // Remove the wire from the list
    QList<WireNet*> netsToDelete;
    for (WireNet* net : _nets) {
        if (net->contains(wire)) {
            net->removeWire(wire);
        }

        if (net->wires().count() < 1) {
            netsToDelete.append(net);
        }
    }

    // Delete the net if this was the nets last wire
    for (WireNet* net : netsToDelete) {
        _nets.removeAll(net);
        delete net;
    }

    return true;
}

QList<Wire*> Scene::wires() const
{
    QList<Wire*> list;

    for (const WireNet* wireNet : _nets) {
        list.append(wireNet->wires());
    }

    return list;
}

QList<WireNet*> Scene::nets() const
{
    return _nets;
}

QList<WireNet*> Scene::nets(const WireNet& wireNet) const
{
    QList<WireNet*> list;

    for (WireNet* net : _nets) {
        if (!net) {
            continue;
        }

        if (net->name().isEmpty())
            continue;

        if (QString::compare(net->name(), wireNet.name(), Qt::CaseInsensitive) == 0) {
            list.append(net);
        }
    }

    return list;
}

WireNet* Scene::net(const Wire& wire) const
{
    for (WireNet* net : _nets) {
        for (const Wire* w : net->wires()) {
            if (w == &wire) {
                return net;
            }
        }
    }

    return nullptr;
}

QList<WireNet*> Scene::netsAt(const QPoint& point)
{
    QList<WireNet*> list;

    for (WireNet* net : _nets) {
        for (const Line& line : net->lineSegments()) {
            if (line.containsPoint(point) && !list.contains(net)) {
                list.append(net);
            }
        }
    }

    return list;
}

void Scene::wireNetHighlightChanged(bool highlighted)
{
    WireNet* wireNet = qobject_cast<WireNet*>(sender());
    if (!wireNet)
        return;

    // Highlight all wire nets that are part of this net
    for (WireNet* otherWireNet : nets(*wireNet)) {
        if (otherWireNet == wireNet) {
            continue;
        }

        otherWireNet->blockSignals(true);
        otherWireNet->setHighlighted(highlighted);
        otherWireNet->blockSignals(false);
    }
}

void Scene::wirePointMoved(Wire& wire, WirePoint& point)
{
    Q_UNUSED(point)

    // Remove the wire from the current net
    // Remove wire nets that have no more wires
    QList<WireNet*> netsToDelete;
    for (WireNet* net : _nets) {
        if (net->contains(wire)) {
            net->removeWire(wire);
            net->setHighlighted(false); // Clear the highlighting
            break;
        }

        if (net->wires().count() <= 0) {
            netsToDelete.append(net);
        }
    }
    for (WireNet* net : netsToDelete) {
        _nets.removeAll(net);
        delete net;
    }

    // If the point is on an existing wire net, add the wire to that net. Otherwise, create a new net
    addWire(&wire);
}

void Scene::showPopup(const Item& item)
{
    _popupInfobox.reset(addWidget(item.popupInfobox()));

    if (_popupInfobox) {
        _popupInfobox->setPos(_lastMousePos + QPointF(5, 5));
        _popupInfobox->setZValue(100);
    }
}

void Scene::addWireNet(WireNet* wireNet)
{
    if (!wireNet) {
        return;
    }

    connect(wireNet, &WireNet::pointMoved, this, &Scene::wirePointMoved);
    connect(wireNet, &WireNet::highlightChanged, this, &Scene::wireNetHighlightChanged);

    _nets.append(wireNet);
}

void Scene::updateWireJunctions()
{
    for (WireNet* net : _nets) {
        net->updateWireJunctions();
    }
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    switch (_mode) {
    case NormalMode:
    {
        // Reset stuff
        _newWire.reset();

        // Handle selection stuff internally (do not remove this!)
        QGraphicsScene::mousePressEvent(event);

        return;
    }

    case WireMode:
    {

        // Left mouse button
        if (event->button() == Qt::LeftButton) {

            QPoint mousetoGridPoint = _settings.toGridPoint(event->scenePos());

            // Start a new line if there isn't already one. Else continue the current one.
            if (!_newWire) {
                _newWire.reset(new Wire);
                _newWire->setAcceptHoverEvents(false);
                addItem(_newWire.data());
            }
            _newWire->appendPoint(mousetoGridPoint);
            _newWireSegment = true;

            return;

        }

        return;
    }

    }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    switch (_mode) {
    case NormalMode:
    {
        // Handle stuff like mouse movement (do not remove this!)
        QGraphicsScene::mouseReleaseEvent(event);

        break;
    }

    case WireMode:
    {
        // Right mouse button
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
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    // Get rid of any popup infobox
    _popupInfobox.reset();

    // Retrieve the new mouse position
    QPointF newMousePos = event->scenePos();

    switch (_mode) {
    case NormalMode:

        // Perform the move
        QGraphicsScene::mouseMoveEvent(event);

        break;

    case WireMode:
    {
        // Make sure that there's a wire
        if (!_newWire) {
            break;
        }

        // Transform mouse coordinates to grid positions (snapped to nearest grid point)
        QPoint mouseGridPos = _settings.toGridPoint(newMousePos);

        // Add a new wire segment. Only allow straight angles (if supposed to)
        if (_settings.routeStraightAngles) {
            if (_newWireSegment) {
                // Remove the last point if there was a previous segment
                if (_newWire->points().count() > 1) {
                    _newWire->removeLastPoint();
                }

                // Create the intermediate point that creates the straight angle
                WirePoint prevNode(_newWire->points().at(_newWire->points().count()-1));
                QPoint corner(prevNode.x(), mouseGridPos.y());
                if (_invertWirePosture) {
                    corner.setX(mouseGridPos.x());
                    corner.setY(prevNode.y());
                }

                // Add the two new points
                _newWire->appendPoint(corner);
                _newWire->appendPoint(mouseGridPos);

                _newWireSegment = false;
            } else {
                // Create the intermediate point that creates the straight angle
                WirePoint prevNode(_newWire->points().at(_newWire->points().count()-3));
                QPoint corner(prevNode.x(), mouseGridPos.y());
                if (_invertWirePosture) {
                    corner.setX(mouseGridPos.x());
                    corner.setY(prevNode.y());
                }

                // Modify the actual wire
                _newWire->movePointTo(_newWire->points().count()-2, corner);
                _newWire->movePointTo(_newWire->points().count()-1, mouseGridPos);
            }
        } else {
            // Don't care about angles and stuff. Fuck geometry, right?
            QPoint newGridPoint = _settings.toGridPoint(newMousePos);
            if (_newWire->points().count() > 1) {
                _newWire->movePointTo(_newWire->points().count()-1, newGridPoint);
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
        // Open the corresponding item editor
        QGraphicsItem* itemUnderCursor = itemAt(event->scenePos(), QTransform());
        if (!itemUnderCursor) {
            break;
        }
        switch (itemUnderCursor->type()) {

        // Component Instance
        case Item::NodeType:
        {
            qDebug("Scene::mouseDoubleClickEvent(): NodeType");
            break;
        }

        // Wire
        case Item::WireType:
        {
            qDebug("Scene::mouseDoubleClickEvent(): WireType");
            break;
        }

        default:
            break;

        }

        return;
    }

    case WireMode:
    {

        // Only do something if there's a wire
        if (_newWire && _newWire->points().count() > 1) {
            bool wireIsFloating = true;

            // Get rid of the last point as mouseDoubleClickEvent() is following mousePressEvent()
            _newWire->removeLastPoint();

            // Check whether the wire was connected to a connector
            for (const QPoint& connectionPoint : connectionPoints()) {
                if (connectionPoint == _newWire->points().last()) {
                    wireIsFloating = false;
                    break;
                }
            }

            // Check wether the wire was connected to another wire
            for (const Wire* wire : wires()) {
                if (wire->pointIsOnWire(_newWire->points().last())) {
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
            _newWire->removeDuplicatePoints();
            connect(_newWire.data(), &Wire::showPopup, this, &Scene::showPopup);
            addWire(_newWire.take());

            return;
        }

        return;
    }

    }
}

void Scene::keyPressEvent(QKeyEvent* event)
{
    Q_UNUSED(event)
}

void Scene::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key()) {

    // Toggle wire posture
    case Qt::Key_Space:
    {
        if (event->modifiers() & Qt::ControlModifier) {
            toggleWirePosture();
        }
    }

    // Change back to normal mode
    case Qt::Key_Escape:
    {
        setMode(NormalMode);
    }
    }
}

void Scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    // Background pen
    QPen backgroundPen;
    backgroundPen.setStyle(Qt::NoPen);

    // Background brush
    QBrush backgroundBrush;
    backgroundBrush.setStyle(Qt::SolidPattern);
    backgroundBrush.setColor(Qt::white);

    // Grid pen
    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(Qt::gray);
    gridPen.setCapStyle(Qt::RoundCap);
    gridPen.setWidth(_settings.gridPointSize);

    // Grid brush
    QBrush gridBrush;
    gridBrush.setStyle(Qt::NoBrush);

    // Draw background
    painter->setPen(backgroundPen);
    painter->setBrush(backgroundBrush);
    painter->drawRect(rect);

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
        painter->setPen(gridPen);
        painter->setBrush(gridBrush);
        painter->drawPoints(points.data(), points.size());
    }
}

QList<QPoint> Scene::connectionPoints() const
{
    QList<QPoint> list;

    for (const QGraphicsItem* graphicsItem : items()) {
        if (graphicsItem->type() == Item::NodeType) {
            const auto node = qgraphicsitem_cast<const Node*>(graphicsItem);
            if (node) {
               list << node->connectionPoints();
            }
        }
    }

    return list;
}
