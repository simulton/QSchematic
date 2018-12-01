#include <QtMath>
#include <QPainter>
#include <QTransform>
#include <QJsonObject>
#include "connector.h"
#include "node.h"
#include "label.h"
#include "../utils.h"

const qreal SIZE               = 1;
const QColor COLOR_BODY_FILL   = QColor(Qt::green);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;
const int TEXT_PADDING         = 8;

using namespace QSchematic;

Connector::Connector(int type, const QPoint& gridPoint, const QString& text, QGraphicsItem* parent) :
    Item(type, parent),
    _snapPolicy(NodeSizerectOutline),
    _forceTextDirection(false),
    _textDirection(Direction::LeftToRight)
{
    // Label
    _label = new Label(this);
    _label->setText(text);

    // Flags
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    // Make sure that we are above the parent
    if (parentItem()) {
        setZValue(parentItem()->zValue() + 1);
    }

    // Connections
    connect(this, &Connector::moved, [this]{ calculateTextDirection(); });

    // Misc
    setGridPos(gridPoint);
    calculateSymbolRect();
    calculateTextDirection();
}

QJsonObject Connector::toJson() const
{
    QJsonObject object;

    object.insert("snap policy", snapPolicy());
    object.insert("force text direction", forceTextDirection());
    object.insert("text direction", textDirection());
    object.insert("label", _label->toJson());

    object.insert("item", Item::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool Connector::fromJson(const QJsonObject& object)
{
    Item::fromJson(object["item"].toObject());

    setSnapPolicy(static_cast<SnapPolicy>(object["snap policy"].toInt()));
    setForceTextDirection(object["force text direction"].toBool());
    _textDirection = static_cast<Direction>(object["text direction"].toInt());
    _label->fromJson(object["label"].toObject());

    return true;
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

void Connector::setLabelIsVisible(bool enabled)
{
    _label->setVisible(enabled);
}

bool Connector::labelIsVisible() const
{
    return _label->isVisible();
}

void Connector::update()
{
    calculateSymbolRect();
    calculateTextDirection();

    Item::update();
}

QPoint Connector::connectionPoint() const
{
    return _settings.toGridPoint(mapToScene(_settings.toScenePoint(QPoint(0, 0))));
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
        QRectF parentNodeSizeRect(0, 0, parentNode->size().width()*_settings.gridSize, parentNode->size().height()*_settings.gridSize);

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

        case NodeShape:
            proposedPos = Utils::clipPointToPath(proposedPos, QPainterPath());
            break;
        }

        // Snap to grid if supposed to
        if (snapToGrid()) {
            return _settings.snapToGridPoint(proposedPos);
        } else {
            return proposedPos;
        }

        break;
    }

    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
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
            const QRect& rect = QRect(0, 0, parentNode->size().width()*_settings.gridSize, parentNode->size().height()*_settings.gridSize);
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

    // Place the label accordingly
    {
        QPointF labelNewPos = _label->pos();
        QTransform t;
        const QRectF& textRect = _label->textRect();

        switch (_textDirection) {
        case LeftToRight:
            labelNewPos.rx() = _symbolRect.x() + _symbolRect.width() + TEXT_PADDING;
            labelNewPos.ry() = _symbolRect.height() - textRect.height() / 2;
            t.rotate(0);
            break;

        case RightToLeft:
            labelNewPos.rx() = _symbolRect.x() - TEXT_PADDING - textRect.width();
            labelNewPos.ry() = _symbolRect.height() - textRect.height() / 2;
            t.rotate(0);
            break;

        case TopToBottom:
            labelNewPos.rx() = _symbolRect.width() - textRect.width() / 2;
            labelNewPos.ry() = _symbolRect.y() + _symbolRect.height() + TEXT_PADDING;
            t.rotate(-90);
            break;

        case BottomToTop:
            labelNewPos.rx() = _symbolRect.width() - textRect.width() / 2;
            labelNewPos.ry() = _symbolRect.y() - TEXT_PADDING;
            t.rotate(-90);
            break;
        }

        _label->setPos(labelNewPos);
        _label->setTransform(t);
    }
}
