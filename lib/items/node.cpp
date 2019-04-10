#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include "node.h"
#include "label.h"
#include "itemfactory.h"
#include "../commands/commandnoderesize.h"
#include "../utils.h"
#include "../scene.h"

#define BOOL2STR(x) (x ? QStringLiteral("true") : QStringLiteral("false"))
#define STR2BOOL(x) (QString::compare(x, QStringLiteral("true")) == 0 ? true : false)

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
    // Label
    _label = std::make_shared<QSchematic::Label>();
    _label->setParentItem(this);
    _label->setVisible(false);
    _label->setMovable(true);
    _label->setText(QStringLiteral("Unnamed"));
}

bool Node::toXml(QXmlStreamWriter& xml) const
{
    xml.writeTextElement(QStringLiteral("width"), QString::number(size().width()));
    xml.writeTextElement(QStringLiteral("height"), QString::number(size().height()));
    xml.writeTextElement(QStringLiteral("resize_policy"), QString::number(mouseResizePolicy()));
    xml.writeTextElement(QStringLiteral("allow_mouse_resize"), BOOL2STR(allowMouseResize()));
    xml.writeTextElement(QStringLiteral("connectors_movable"), BOOL2STR(connectorsMovable()));
    xml.writeTextElement(QStringLiteral("connectors_snap_policy"), QString::number(connectorsSnapPolicy()));
    xml.writeTextElement(QStringLiteral("connectors_snap_to_grid"), BOOL2STR(connectorsSnapToGrid()));
    xml.writeStartElement(QStringLiteral("label"));
        label()->toXml(xml);
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("connectors"));
    for (const auto& connector : connectors()) {
        xml.writeStartElement(QStringLiteral("connector"));
        connector->toXml(xml);
        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("item"));
    addTypeIdentifierToXml(xml);
    Item::toXml(xml);
    xml.writeEndElement();

    return true;
}

bool Node::fromXml(QXmlStreamReader& reader)
{
    int width = 0;
    int height = 0;
    while (reader.readNextStartElement()) {
        if (reader.name() == "item") {
            Item::fromXml(reader);
        } else if (reader.name() == "width") {
            width = reader.readElementText().toInt();
        } else if (reader.name() == "height") {
            height = reader.readElementText().toInt();
        } else if (reader.name() == "resize_policy") {
            setMouseResizePolicy(static_cast<ResizePolicy>(reader.readElementText().toInt()));
        } else if (reader.name() == "allow_mouse_resize") {
            setAllowMouseResize(STR2BOOL(reader.readElementText()));
        } else if (reader.name() == "connectors_movable") {
            setConnectorsMovable(STR2BOOL(reader.readElementText()));
        } else if (reader.name() == "connectors_snap_policy") {
            setConnectorsSnapPolicy(static_cast<Connector::SnapPolicy>(reader.readElementText().toInt()));
        } else if (reader.name() == "connectors_snap_to_grid") {
            setConnectorsSnapToGrid(STR2BOOL(reader.readElementText()));
        } else if (reader.name() == "label") {
            label()->fromXml(reader);
        } else if (reader.name() == "connectors") {
            clearConnectors();
            while (reader.readNextStartElement()) {
                if (reader.name() != "connector") {
                    reader.skipCurrentElement();
                    continue;
                }
                Connector* connector = dynamic_cast<Connector*>(ItemFactory::instance().fromXml(reader).release());
                if (!connector) {
                    continue;
                }
                connector->fromXml(reader);
                addConnector(std::unique_ptr<Connector>(connector));
            }
        }
    }

    setSize(width, height);

    return true;
}

std::unique_ptr<Item> Node::deepCopy() const
{
    auto clone = std::make_unique<Node>(type(), parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void Node::copyAttributes(Node& dest) const
{
    // Base class
    Item::copyAttributes(dest);

    // Label
    auto labelClone = qgraphicsitem_cast<Label*>(_label->deepCopy().release());
    dest._label = std::shared_ptr<Label>(labelClone);
    dest._label->setParentItem(&dest);

    // Connectors
    dest.clearConnectors();
    for (const auto& connector : _connectors) {
        auto connectorClone = qgraphicsitem_cast<Connector*>(connector->deepCopy().release());
        connectorClone->setParentItem(&dest);
        dest._connectors << std::shared_ptr<Connector>(connectorClone);
    }

    // Attributes
    dest._mode = _mode;
    dest._lastMousePosWithGridMove = _lastMousePosWithGridMove;
    dest._resizeHandle = _resizeHandle;
    dest._size = _size;
    dest._mouseResizePolicy = _mouseResizePolicy;
    dest._allowMouseResize = _allowMouseResize;
    dest._connectorsMovable = _connectorsMovable;
    dest._connectorsSnapPolicy = _connectorsSnapPolicy;
    dest._connectorsSnapToGrid = _connectorsSnapToGrid;
}

Node::Mode Node::mode() const
{
    return _mode;
}

void Node::setSize(const QSize& size)
{
    // Boundary checks
    if (size.width() < 1 or size.height() < 1) {
        return;
    }

    prepareGeometryChange();

    _size = size;

    emit sizeChanged();
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

bool Node::addConnector(const std::shared_ptr<Connector>& connector)
{
    if (!connector) {
        return false;
    }

    connector->setParentItem(this);
    connector->setVisible(true);
    connector->setMovable(_connectorsMovable);
    connector->setSnapPolicy(_connectorsSnapPolicy);
    connector->setSnapToGrid(_connectorsSnapToGrid);

    _connectors << connector;

    return true;
}

bool Node::removeConnector(const std::shared_ptr<Connector>& connector)
{
    if (!connector or !_connectors.contains(connector)) {
        return false;
    }

    connector->setParentItem(nullptr);
    connector->setVisible(false);

    _connectors.removeAll(connector);

    return true;
}

void Node::clearConnectors()
{
    // Remove from scene
    auto s = scene();
    if (s) {
        for (auto connector : _connectors) {
            s->removeItem(connector);
        }
    }

    // Clear the local list
    _connectors.clear();
}

QList<std::shared_ptr<Connector>> Node::connectors() const
{
    return _connectors;
}

QList<QPointF> Node::connectionPointsRelative() const
{
    QList<QPointF> list;

    for (const auto& connector : _connectors) {
        list << connector->connectionPoint() + connector->pos();
    }

    return list;
}

QList<QPointF> Node::connectionPointsAbsolute() const
{
    QList<QPointF> list(connectionPointsRelative());

    for (QPointF& point : list) {
        point += pos();
    }

    return list;
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

std::shared_ptr<Label> Node::label() const
{
    return _label;
}


void Node::setText(const QString& text)
{
    Q_ASSERT(_label);

    _label->setText(text);
}

QString Node::text() const
{
    Q_ASSERT(_label);

    return _label->text();
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

            // Apply
            if (scene()) {
                scene()->undoStack()->push(new CommandNodeResize(this, QPoint(newX, newY), QSize(newWidth, newHeight)));
            }
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
