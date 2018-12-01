#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include "node.h"
#include "itemfactory.h"
#include "../utils.h"
#include "../scene.h"

const QColor COLOR_HIGHLIGHTED = QColor(Qt::blue).lighter();
const QColor COLOR_BODY_FILL   = QColor(Qt::green);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

using namespace QSchematic;

const int DEFAULT_WIDTH     = 8;
const int DEFAULT_HEIGHT    = 12;

Node::Node(int type, QGraphicsItem* parent) :
    Item(type, parent),
    _mode(None),
    _size(DEFAULT_WIDTH, DEFAULT_HEIGHT),
    _mouseResizePolicy(static_cast<ResizePolicy>(0)),
    _allowMouseResize(true),
    _connectorsMovable(false),
    _connectorsSnapPolicy(Connector::NodeSizerectOutline),
    _connectorsSnapToGrid(true)
{
}

QJsonObject Node::toJson() const
{
    QJsonObject object;

    object.insert("size width", size().width());
    object.insert("size height", size().height());
    object.insert("mouse resize policy", mouseResizePolicy());
    object.insert("allow mouse resize", allowMouseResize());
    object.insert("connectors movable", connectorsMovable());
    object.insert("connectors snap policy", connectorsSnapPolicy());
    object.insert("connectors snap to grid", connectorsSnapToGrid());

    QJsonArray connectorsArray;
    for (const Connector* connector : connectors()) {
        connectorsArray << connector->toJson();
    }
    object.insert("connectors", connectorsArray);

    object.insert("item", Item::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool Node::fromJson(const QJsonObject& object)
{
    Item::fromJson(object["item"].toObject());

    setSize(object["size width"].toInt(), object["size height"].toInt());
    setMouseResizePolicy(static_cast<ResizePolicy>(object["mouse resize policy"].toInt()));
    setAllowMouseResize(object["allow mouse resize"].toBool());
    setConnectorsMovable(object["connectors movable"].toBool());
    setConnectorsSnapPolicy(static_cast<Connector::SnapPolicy>(object["connectors snap policy"].toInt()));
    setConnectorsSnapToGrid(object["connectors snap to grid"].toBool());

    for (const QJsonValue& value : object["connectors"].toArray()) {
        QJsonObject object = value.toObject();
        if (!object.isEmpty()) {
            Connector* connector = dynamic_cast<Connector*>(ItemFactory::instance().fromJson(object).release());
            if (!connector) {
                continue;
            }
            connector->fromJson(object);
            addConnector(connector);
        }
    }

    return true;
}

void Node::setSize(const QSize& size)
{
    // Boundary checks
    if (size.width() < 1 or size.height() < 1) {
        return;
    }

    prepareGeometryChange();

    _size = size;

    emit sizeChanged(_size);
}

void Node::setSize(int width, int height)
{
    return setSize(QSize(width, height));
}

QSize Node::size() const
{
    return _size;
}

QRect Node::sizeRect() const
{
    return QRect(0, 0, _size.width(), _size.height());
}

QRect Node::sizeSceneRect() const
{
    return QRect(0, 0, _size.width()*_settings.gridSize, _size.height()*_settings.gridSize);
}

void Node::setMouseResizePolicy(ResizePolicy policy)
{
    _mouseResizePolicy = policy;
}

Node::ResizePolicy Node::mouseResizePolicy() const
{
    return _mouseResizePolicy;
}

void Node::setAllowMouseResize(bool enabled)
{
    _allowMouseResize = enabled;
}

bool Node::allowMouseResize() const
{
    return _allowMouseResize;
}

QMap<RectanglePoint, QRect> Node::resizeHandles() const
{
    QMap<RectanglePoint, QRect> map;
    const int& resizeHandleSize = _settings.resizeHandleSize;

    QRect r(QPoint(0, 0), _size*_settings.gridSize);

    // Corners
    map.insert(RectanglePointBottomRight, QRect(r.bottomRight()+QPoint(1,1)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));
    map.insert(RectanglePointBottomLeft, QRect(r.bottomLeft()+QPoint(1,1)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));
    map.insert(RectanglePointTopRight, QRect(r.topRight()+QPoint(1,1)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));
    map.insert(RectanglePointTopLeft, QRect(r.topLeft()+QPoint(1,1)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));

    // Sides
    if (r.topRight().x() - r.topLeft().x() > 7*resizeHandleSize) {
        map.insert(RectanglePointTop, QRect(Utils::centerPoint(r.topRight(), r.topLeft())+QPoint(1,1)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));
        map.insert(RectanglePointBottom, QRect(Utils::centerPoint(r.bottomRight(), r.bottomLeft())+QPoint(1,1)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));
    }
    if (r.bottomLeft().y() - r.topLeft().y() > 7*resizeHandleSize) {
        map.insert(RectanglePointRight, QRect(Utils::centerPoint(r.topRight(), r.bottomRight())+QPoint(1,0)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));
        map.insert(RectanglePointLeft, QRect(Utils::centerPoint(r.bottomLeft(), r.topLeft())+QPoint(1,0)-QPoint(resizeHandleSize, resizeHandleSize), QSize(2*resizeHandleSize, 2*resizeHandleSize)));
    }

    return map;
}

bool Node::addConnector(Connector* connector)
{
    if (!connector) {
        return false;
    }

    connector->setParentItem(this);
    connector->setMovable(_connectorsMovable);
    connector->setSnapPolicy(_connectorsSnapPolicy);

    _connectors << connector;

    return true;
}

bool Node::removeConnector(const QPoint& point)
{
    for (auto it = _connectors.begin(); it != _connectors.end(); it++) {
        Q_ASSERT(*it);

        if (mapFromItem(*it, (*it)->connectionPoint()) == point) {
            _connectors.removeAll(*it);
            delete *it;

            return true;
        }
    }

    return false;
}

void Node::clearConnectors()
{
    qDeleteAll(_connectors);
    _connectors.clear();
}

QList<Connector*> Node::connectors() const
{
    return _connectors;
}

QList<QPoint> Node::connectionPoints() const
{
    QList<QPoint> list;

    for (const auto& connector : _connectors) {
        list << connector->connectionPoint();
    }

    return list;
}

bool Node::isConnectionPoint(const QPoint& gridPoint) const
{
    return connectionPoints().contains(gridPoint);
}

void Node::setConnectorsMovable(bool enabled)
{
    // Update connectors
    for (auto connector : _connectors) {
        connector->setMovable(enabled);
    }

    // Update local
    _connectorsMovable = enabled;
}

bool Node::connectorsMovable() const
{
    return _connectorsMovable;
}

void Node::setConnectorsSnapPolicy(Connector::SnapPolicy policy)
{
    // Update connectors
    for (auto connector : _connectors) {
        connector->setSnapPolicy(policy);
    }

    // Update local
    _connectorsSnapPolicy = policy;
}

Connector::SnapPolicy Node::connectorsSnapPolicy() const
{
    return _connectorsSnapPolicy;
}

void Node::setConnectorsSnapToGrid(bool enabled)
{
    // Update connectors
    for (auto connector : _connectors) {
        connector->setSnapToGrid(enabled);
    }

    // Update local
    _connectorsSnapToGrid = enabled;
}

bool Node::connectorsSnapToGrid() const
{
    return _connectorsSnapToGrid;
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    // Let the base class handle selection and so on
    Item::mousePressEvent(event);

    // Presume no mode
    _mode = None;

    // Check if clicked on a resize handle
    if (isSelected() && _allowMouseResize) {
        auto handles = resizeHandles();
        auto it = handles.constBegin();
        while (it != handles.constEnd()) {
            if (it.value().contains(event->pos().toPoint())) {
                _mode = Resize;
                _lastMousePosWithGridMove = _settings.toGridPoint(event->scenePos());
                _resizeHandle = it.key();
                break;
            }
            it++;
        }
    }
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    Item::mouseReleaseEvent(event);

    _mode = None;
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    QPoint newMouseGridPos(_settings.toGridPoint(event->scenePos()));

    switch (_mode) {
    case None:
    {
        Item::mouseMoveEvent(event);

        break;
    }

    case Resize:
    {
        // Sanity check
        if (!_allowMouseResize) {
            qFatal("Node::mouseMoveEvent(): _mode is 'Resize' although _allowMouseResize is false");
            break;
        }

        // Left mouse button to move
        if (event->buttons() & Qt::LeftButton) {

            // Calculate mouse movement in grid units
            QPoint d = newMouseGridPos - _lastMousePosWithGridMove;
            int dx = d.x();
            int dy = d.y();

            // Don't do anything if there's nothing to do
            if (dx == 0 and dy == 0) {
                break;
            }

            // Honor mouse resize policy
            if (_mouseResizePolicy != 0) {
                if (qAbs(dx % 2) != 0 or qAbs(dy % 2) != 0) {
                    break;
                }
            }

            // Track this
            _lastMousePosWithGridMove = newMouseGridPos;

            // Perform resizing
            int newX = gridPosX();
            int newY = gridPosY();
            int newWidth = _size.width();
            int newHeight = _size.height();
            switch (_resizeHandle) {
            case RectanglePointTopLeft:
                newX += dx;
                newY += dy;
                newWidth -= dx;
                newHeight -= dy;
                break;

            case RectanglePointTop:
                newY += dy;
                newHeight -= dy;
                break;

            case RectanglePointTopRight:
                newY += dy;
                newWidth += dx;
                newHeight -= dy;
                break;

            case RectanglePointRight:
                newWidth += dx;
                break;

            case RectanglePointBottomRight:
                newWidth += dx;
                newHeight += dy;
                break;

            case RectanglePointBottom:
                newHeight += dy;
                break;

            case RectanglePointBottomLeft:
                newX += dx;
                newWidth -= dx;
                newHeight += dy;
                break;

            case RectanglePointLeft:
                newX += dx;
                newWidth -= dx;
                break;
            }

            // Set new size & position
            setSize(newWidth, newHeight);
            setGridPos(newX, newY);
        }

        break;
    }
    }
}

void Node::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Item::hoverEnterEvent(event);
}

void Node::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Item::hoverLeaveEvent(event);

    unsetCursor();
}

void Node::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    Item::hoverMoveEvent(event);

    // Set the cursor
    {
        setCursor(Qt::ArrowCursor);

        // If selected, we should adjust the cursor for the resize handles
        if (isSelected() and _allowMouseResize) {
            auto handles = resizeHandles();
            auto it = handles.constBegin();
            while (it != handles.constEnd()) {
                if (it.value().contains(event->pos().toPoint())) {
                    switch (it.key()) {
                    case RectanglePointTopLeft:
                    case RectanglePointBottomRight:
                        setCursor(Qt::SizeFDiagCursor);
                        break;

                    case RectanglePointBottom:
                    case RectanglePointTop:
                        setCursor(Qt::SizeVerCursor);
                        break;

                    case RectanglePointBottomLeft:
                    case RectanglePointTopRight:
                        setCursor(Qt::SizeBDiagCursor);
                        break;

                    case RectanglePointRight:
                    case RectanglePointLeft:
                        setCursor(Qt::SizeHorCursor);
                        break;
                    }
                    break;
                }
                it++;
            }
        }
    }
}

QRectF Node::boundingRect() const
{
    QRectF rect;
    qreal adj = 0.0;

    // Body rect
    rect = rect.united(QRectF(QPoint(0, 0), _size*_settings.gridSize));

    // Add half the pen width
    adj = qMax(adj, PEN_WIDTH / 2.0);

    // Add resize handles
    if (isSelected() and _allowMouseResize) {
        adj = qMax(adj, static_cast<qreal>(_settings.resizeHandleSize));
    }

    // Add highlight rect
    if (isHighlighted()) {
        adj = qMax(adj, static_cast<qreal>(_settings.highlightRectPadding));
    }

    return rect.adjusted(-adj, -adj, adj, adj);
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Draw the bounding rect if debug mode is enabled
    if (_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawRect(boundingRect());
    }

    // Highlight rectangle
    if (isHighlighted()) {
        // Highlight pen
        QPen highlightPen;
        highlightPen.setStyle(Qt::NoPen);

        // Highlight brush
        QBrush highlightBrush;
        highlightBrush.setStyle(Qt::SolidPattern);
        highlightBrush.setColor(COLOR_HIGHLIGHTED);

        // Highlight rectangle
        painter->setPen(highlightPen);
        painter->setBrush(highlightBrush);
        painter->setOpacity(0.5);
        int adj = _settings.highlightRectPadding;
        painter->drawRoundedRect(QRect(QPoint(0, 0), _size*_settings.gridSize).adjusted(-adj, -adj, adj, adj), _settings.gridSize/2, _settings.gridSize/2);
    }

    painter->setOpacity(1.0);

    // Body pen
    QPen bodyPen;
    bodyPen.setWidthF(PEN_WIDTH);
    bodyPen.setStyle(Qt::SolidLine);
    bodyPen.setColor(COLOR_BODY_BORDER);

    // Body brush
    QBrush bodyBrush;
    bodyBrush.setStyle(Qt::SolidPattern);
    bodyBrush.setColor(COLOR_BODY_FILL);

    // Draw the component body
    painter->setPen(bodyPen);
    painter->setBrush(bodyBrush);
    painter->drawRoundedRect(QRect(QPoint(0, 0), _size*_settings.gridSize), _settings.gridSize/2, _settings.gridSize/2);

    // Resize handles
    if (isSelected() and allowMouseResize()) {
        paintResizeHandles(*painter);
    }
}

void Node::paintResizeHandles(QPainter& painter)
{
    for (const QRect& rect : resizeHandles()) {
        // Handle pen
        QPen handlePen;
        handlePen.setStyle(Qt::NoPen);
        painter.setPen(handlePen);

        // Handle Brush
        QBrush handleBrush;
        handleBrush.setStyle(Qt::SolidPattern);
        painter.setBrush(handleBrush);

        // Draw the outer handle
        handleBrush.setColor("#3fa9f5");
        painter.setBrush(handleBrush);
        painter.drawRect(rect.adjusted(-handlePen.width(), -handlePen.width(), handlePen.width()/2, handlePen.width()/2));

        // Draw the inner handle
        int adj = _settings.resizeHandleSize/2;
        handleBrush.setColor(Qt::white);
        painter.setBrush(handleBrush);
        painter.drawRect(rect.adjusted(-handlePen.width()+adj, -handlePen.width()+adj, (handlePen.width()/2)-adj, (handlePen.width()/2)-adj));
    }
}
