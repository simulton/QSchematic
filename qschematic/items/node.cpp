#include "node.h"
#include "itemfactory.h"
#include "../utils.h"
#include "../scene.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtMath>

const QColor COLOR_HIGHLIGHTED = QColor(Qt::blue);
const QColor COLOR_BODY_FILL   = QColor(Qt::green);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

using namespace QSchematic;

const int DEFAULT_WIDTH     = 160;
const int DEFAULT_HEIGHT    = 240;

Node::Node(int type, QGraphicsItem* parent) :
    RectItem(type, parent),
    _connectorsMovable(false),
    _connectorsSnapPolicy(Connector::NodeSizerectOutline),
    _connectorsSnapToGrid(true)
{
    connect(this, &Node::settingsChanged, this, &Node::propagateSettings);
}

Node::~Node()
{
    dissociate_items(_connectors);
    dissociate_items(_specialConnectors);
}

gpds::container Node::to_container() const
{
    // Connectors configuration
    gpds::container connectorsConfigurationContainer;
    connectorsConfigurationContainer.add_value("movable", connectorsMovable());
    connectorsConfigurationContainer.add_value("snap_policy", static_cast<int>(connectorsSnapPolicy()));
    connectorsConfigurationContainer.add_value("snap_to_grid", connectorsSnapToGrid());

    // Connectors
    gpds::container connectorsContainer;
    for (const auto& connector : connectors()) {
        if ( _specialConnectors.contains( connector ) ) {
            continue;
        }

        connectorsContainer.add_value("connector", connector->to_container());
    }

    // Root
    gpds::container root;
    addItemTypeIdToContainer(root);
    root.add_value("rect_item", RectItem::to_container());
    root.add_value("width", size().width());
    root.add_value("height", size().height());
    root.add_value("allow_mouse_resize", allowMouseResize());
    root.add_value("allow_mouse_rotate", allowMouseRotate());
    root.add_value("connectors_configuration", connectorsConfigurationContainer);
    root.add_value("connectors", connectorsContainer);

    return root;
}

void Node::from_container(const gpds::container& container)
{
    // Root
    RectItem::from_container(*container.get_value<gpds::container*>("rect_item").value());
    setSize(container.get_value<double>("width").value_or(0), container.get_value<double>("height").value_or(0));
    setAllowMouseResize(container.get_value<bool>("allow_mouse_resize").value_or(true));
    setAllowMouseRotate(container.get_value<bool>("allow_mouse_rotate").value_or(true));

    // Connectors configuration
    const gpds::container* connectorsConfigurationContainer = container.get_value<gpds::container*>("connectors_configuration").value_or(nullptr);
    if (connectorsConfigurationContainer) {
        setConnectorsMovable(connectorsConfigurationContainer->get_value<bool>("movable").value_or(true));
        setConnectorsSnapPolicy(static_cast<Connector::SnapPolicy>( connectorsConfigurationContainer->get_value<int>("snap_policy").value_or(Connector::SnapPolicy::Anywhere)));
        setConnectorsSnapToGrid(connectorsConfigurationContainer->get_value<bool>("snap_to_grid").value_or(true));
    }

    // Connectors
    const gpds::container* connectorsContainer = container.get_value<gpds::container*>("connectors").value_or(nullptr);
    if (connectorsContainer) {
        clearConnectors();
        for (const gpds::container* connectorContainer : connectorsContainer->get_values<gpds::container*>("connector")) {
            auto connector = std::dynamic_pointer_cast<Connector>(ItemFactory::instance().from_container(*connectorContainer));
            if (!connector) {
                continue;
            }
            connector->from_container(*connectorContainer);
            addConnector(connector);
        }
    }
}

std::shared_ptr<Item> Node::deepCopy() const
{
    auto clone = std::make_shared<Node>(type(), parentItem());
    copyAttributes(*clone);

    return clone;
}

void Node::copyAttributes(Node& dest) const
{
    // Base class
    RectItem::copyAttributes(dest);

    // Connectors
    dest.clearConnectors();
    for (const auto& connector : _connectors) {
        if ( _specialConnectors.contains( connector ) ) {
            continue;
        }

        auto connectorClone = std::dynamic_pointer_cast<Connector>(connector->deepCopy());
        connectorClone->setParentItem(&dest);
        dest._connectors << connectorClone;
    }

    // Attributes
    dest._connectorsMovable = _connectorsMovable;
    dest._connectorsSnapPolicy = _connectorsSnapPolicy;
    dest._connectorsSnapToGrid = _connectorsSnapToGrid;
    dest._specialConnectors = _specialConnectors;
}

void Node::addSpecialConnector(const std::shared_ptr<Connector>& connector)
{
    // Sanity check
    if (!connector)
        return;

    _specialConnectors.push_back( connector );

    addConnector( connector );
}

bool Node::addConnector(const std::shared_ptr<Connector>& connector)
{
    if (!connector) {
        return false;
    }

    connector->setParentItem(this);
    connector->setMovable(_connectorsMovable);
    connector->setSnapPolicy(_connectorsSnapPolicy);
    connector->setSnapToGrid(_connectorsSnapToGrid);
    connector->setSettings(_settings);

    _connectors << connector;

    return true;
}

bool Node::removeConnector(const std::shared_ptr<Connector>& connector)
{
    if (!connector) {
        return false;
    }
    if (!_connectors.contains(connector) && !_specialConnectors.contains(connector)) {
        return false;
    }

    connector->setParentItem(nullptr);

    _connectors.removeAll(connector);
    _specialConnectors.removeAll(connector);

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
        // Ignore hidden connectors
        if (!connector->isVisible())
            continue;

        // Rotate the position around to the node's origin
        QPointF pos = connector->pos();
        {
            QPointF d = transformOriginPoint() - pos;
            qreal angle = rotation() * M_PI / 180;
            QPointF rotated;
            rotated.setX(qCos(angle) * d.rx() - qSin(angle) * d.ry());
            rotated.setY(qSin(angle) * d.rx() + qCos(angle) * d.ry());
            pos = transformOriginPoint() - rotated;
        }
        list << connector->connectionPoint() + pos;
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

void Node::alignConnectorLabels() const
{
    for (auto connector : _connectors) {
        Q_ASSERT(connector);
        connector->alignLabel();
    }
}

void Node::sizeChangedEvent(const QSizeF oldSize, const QSizeF newSize)
{
    for (const auto& connector : connectors()) {
        if (qFuzzyCompare(connector->posX(), oldSize.width()) || connector->posX() > newSize.width())
            connector->setX(newSize.width());

        if (qFuzzyCompare(connector->posY(), oldSize.height()) || connector->posY() > newSize.height())
            connector->setY(newSize.height());
    }
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
        painter->drawRoundedRect(sizeRect().adjusted(-adj, -adj, adj, adj), _settings.gridSize/2, _settings.gridSize/2);
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
    painter->drawRoundedRect(sizeRect(), _settings.gridSize/2, _settings.gridSize/2);

    // Resize handles
    if (isSelected() && allowMouseResize()) {
        paintResizeHandles(*painter);
    }

    // Rotate handle
    if (isSelected() && allowMouseRotate()) {
        paintRotateHandle(*painter);
    }
}

void Node::update()
{
    // The item class sets the origin to the center of the bounding box
    // but in this case we want to rotate around the center of the body
    setTransformOriginPoint(sizeRect().center());
    QGraphicsObject::update();
}

void Node::propagateSettings()
{
    for (const auto& connector : connectors()) {
        connector->setSettings(_settings);
    }
}
