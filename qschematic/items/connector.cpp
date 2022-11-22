#include "connector.h"
#include "label.h"
#include "node.h"
#include "wire.h"
#include "../scene.h"
#include "../utils.h"

#include <QtMath>
#include <QPainter>
#include <QTransform>
#include <QVector2D>
#include <QGraphicsSceneHoverEvent>

const qreal SIZE               = 1;
const QColor COLOR_BODY_FILL   = QColor(Qt::green);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;
const int TEXT_PADDING         = 15;

using namespace QSchematic;

Connector::Connector(int type, const QPoint& gridPoint, const QString& text, QGraphicsItem* parent) :
    Item(type, parent),
    _snapPolicy(NodeSizerectOutline),
    _forceTextDirection(false),
    _textDirection(Direction::LeftToRight)
{
    // Label
    _label = std::make_shared<Label>();
    _label->setParentItem(this);
    _label->setText(text);

    // Flags
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    // Make sure that we are above the parent
    if (parentItem()) {
        setZValue(parentItem()->zValue() + 1);
    }

    // Connections
    connect(this, &Connector::moved, [this]{ calculateTextDirection(); });
    connect(this, &Connector::movedInScene, this, &Connector::notify_wire_manager);

    // Misc
    setGridPos(gridPoint);
    calculateSymbolRect();
    calculateTextDirection();
}

Connector::~Connector()
{
    // So it's definitely removed via the shared_ptr (which we have by way of the item-allocation contracts being shptr all through
    dissociate_item(_label);

    // Disconnect all wires
    disconnect_all_wires();
}

gpds::container Connector::to_container() const
{
    // Root
    gpds::container root;
    addItemTypeIdToContainer(root);
    root.add_value("item", Item::to_container());
    root.add_value("snap_policy", snapPolicy());
    root.add_value("force_text_direction", forceTextDirection());
    root.add_value("text_direction", textDirection());
    root.add_value("label", _label->to_container());

    return root;
}

void Connector::from_container(const gpds::container& container)
{
    Item::from_container(*container.get_value<gpds::container*>("item").value());
    setSnapPolicy(static_cast<SnapPolicy>(container.get_value<int>("snap_policy").value_or(Anywhere)));
    setForceTextDirection(container.get_value<bool>("force_text_direction").value_or(false));
    _textDirection = static_cast<Direction>(container.get_value<int>("text_direction").value_or(LeftToRight));
    _label->from_container(*container.get_value<gpds::container*>("label").value());
}

std::shared_ptr<Item> Connector::deepCopy() const
{
    auto clone = std::make_shared<Connector>(type(), gridPos(), text(), parentItem());
    copyAttributes(*clone);

    return clone;
}

void Connector::copyAttributes(Connector& dest) const
{
    Q_ASSERT(_label);

    // Base class
    Item::copyAttributes(dest);

    // Label
    dest._label = std::dynamic_pointer_cast<QSchematic::Label>(_label->deepCopy());
    dest._label->setParentItem(&dest);

    // Attributes
    dest._snapPolicy = _snapPolicy;
    dest._symbolRect = _symbolRect;
    dest._forceTextDirection = _forceTextDirection;
    dest._textDirection = _textDirection;
}

void Connector::setSnapPolicy(Connector::SnapPolicy policy)
{
    _snapPolicy = policy;
}

Connector::SnapPolicy Connector::snapPolicy() const
{
    return _snapPolicy;
}

void Connector::setText(const QString& text)
{
    _label->setText(text);

    calculateTextDirection();
}

QString Connector::text() const
{
    return _label->text();
}

void Connector::setForceTextDirection(bool enabled)
{
    _forceTextDirection = enabled;
}

bool Connector::forceTextDirection() const
{
    return _forceTextDirection;
}

void Connector::setForcedTextDirection(Direction direction)
{
    _textDirection = direction;

    update();
}

Direction Connector::textDirection() const
{
    return _textDirection;
}

void Connector::update()
{
    calculateSymbolRect();
    calculateTextDirection();

    Item::update();
}

QPointF Connector::connectionPoint() const
{
    return QPointF(0, 0);
}

QRectF Connector::boundingRect() const
{
    qreal adj = qCeil(PEN_WIDTH / 2.0);
    if (isHighlighted()) {
        adj += _settings.highlightRectPadding;
    }

    return _symbolRect.adjusted(-adj, -adj, adj, adj);
}

QVariant Connector::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
    // Snap to whatever we're supposed to snap to
    case QGraphicsItem::ItemPositionChange:
    {
        QPointF proposedPos = value.toPointF();

        // Retrieve parent Node's size rect
        const Node* parentNode = qgraphicsitem_cast<const Node*>(parentItem());
        if (!parentNode) {
            return proposedPos;
        }
        QRectF parentNodeSizeRect(0, 0, parentNode->size().width(), parentNode->size().height());

        // Honor snap policy
        switch (_snapPolicy) {
        case Anywhere:
            break;

        case NodeSizerect:
            proposedPos = Utils::clipPointToRect(proposedPos, parentNodeSizeRect);
            break;

        case NodeSizerectOutline:
            proposedPos = Utils::clipPointToRectOutline(proposedPos, parentNodeSizeRect);
            break;
        }

        // Honor snap-to-grid
        if (parentNode->canSnapToGrid() && snapToGrid()) {
            proposedPos = _settings.snapToGrid(proposedPos);
        }

        return proposedPos;
    }

    case QGraphicsItem::ItemPositionHasChanged:
    case QGraphicsItem::ItemParentHasChanged:
    {
        calculateTextDirection();
        alignLabel();
        break;
    }

    case QGraphicsItem::ItemVisibleHasChanged:
    {
        if (!isVisible())
            disconnect_all_wires();
        break;
    }

    case QGraphicsItem::ItemSceneChange:
    {
        disconnect_all_wires();
        break;
    }

    default:
        break;
    }

    return Item::itemChange(change, value);
}

void Connector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Draw the bounding rect if debug mode is enabled
    if (_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawRect(boundingRect());
    }

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
    painter->drawRoundedRect(_symbolRect, _settings.gridSize/4, _settings.gridSize/4);
}

std::shared_ptr<Label> Connector::label() const
{
    return _label;
}

void Connector::alignLabel()
{
    QPointF labelNewPos = _label->pos();
    QTransform t;
    const QRectF& textRect = _label->textRect();

    switch (_textDirection) {
        case LeftToRight:
            labelNewPos.rx() = TEXT_PADDING;
            labelNewPos.ry() = textRect.height()/4;
            t.rotate(0);
            break;

        case RightToLeft:
            labelNewPos.rx() = -textRect.width() - TEXT_PADDING;
            labelNewPos.ry() = textRect.height()/4;
            t.rotate(0);
            break;

        case TopToBottom:
            labelNewPos.rx() = textRect.height()/4;
            labelNewPos.ry() = textRect.width() + TEXT_PADDING;
            t.rotate(-90);
            break;

        case BottomToTop:
            labelNewPos.rx() = textRect.height()/4;
            labelNewPos.ry() = - TEXT_PADDING;
            t.rotate(-90);
            break;
    }

    _label->setPos(labelNewPos);
    _label->setTransform(t);
}

void Connector::calculateSymbolRect()
{
    _symbolRect = QRectF(-SIZE*_settings.gridSize/2.0, -SIZE*_settings.gridSize/2.0, SIZE*_settings.gridSize, SIZE*_settings.gridSize);
}

void Connector::calculateTextDirection()
{
    // Honor forced override
    if (_forceTextDirection) {
        return;
    }

    // Nothing to do if there's no text
    if (text().isEmpty()) {
        _textDirection = LeftToRight;
        return;
    }

    // Figure out the text direction
    {
        _textDirection = LeftToRight;
        const Node* parentNode = qgraphicsitem_cast<const Node*>(parentItem());
        if (parentNode) {

            // Create list of edges
            QVector<QLineF> edges(4);
            const QRectF& rect = parentNode->sizeRect();
            edges[0] = QLineF(rect.topLeft(), rect.topRight());
            edges[1] = QLineF(rect.topRight(), rect.bottomRight());
            edges[2] = QLineF(rect.bottomRight(), rect.bottomLeft());
            edges[3] = QLineF(rect.bottomLeft(), rect.topLeft());

            // Figure out which edge we're closest to
            auto closestEdgeIterator = Utils::lineClosestToPoint(edges, pos());
            int edgeIndex = closestEdgeIterator - edges.constBegin();

            // Set the correct text direction
            switch (edgeIndex) {
            case 0:
                _textDirection = TopToBottom;
                break;

            case 1:
                _textDirection = RightToLeft;
                break;

            case 2:
                _textDirection = BottomToTop;
                break;

            case 3:
            default:
                _textDirection = LeftToRight;
                break;
            }
        }
    }
}

QPointF Connector::position() const
{
    return scenePos();
}

void Connector::disconnect_all_wires()
{
    auto s = scene();
    if (!s)
        return;

    auto wireManager = s->wire_manager();
    if (!wireManager)
        return;

    wireManager->detach_wire(this);
}

void Connector::notify_wire_manager()
{
    auto s = scene();
    if (!s)
        return;

    auto wireManager = s->wire_manager();
    if (!wireManager)
        return;

    // Notify the wire system when the connector moves
    wireManager->connector_moved(this);
}
