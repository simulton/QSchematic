#include <QtMath>
#include <QPainter>
#include <QFontMetrics>
#include "connector.h"
#include "node.h"
#include "../utils.h"

const qreal SIZE               = 1;
const QColor COLOR_BODY_FILL   = QColor(Qt::green);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;
const int TEXT_PADDING         = 10;

using namespace QSchematic;

Connector::Connector(QGraphicsItem* parent) :
    Item(Item::ConnectorType, parent),
    _snapPolicy(NodeSizerectOutline),
    _textDirection(Direction::LeftToRight)
{
    // Flags
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    // Make sure that we are above the parent
    if (parentItem()) {
        setZValue(parentItem()->zValue() + 1);
    }

    // Connections
    connect(this, &Connector::moved, [this]{ calculateTextRect(); });

    // Misc
    calculateSymbolRect();
    calculateTextRect();
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
    _text = text;

    calculateTextRect();
    update();
}

QString Connector::text() const
{
    return _text;
}

void Connector::update()
{
    calculateSymbolRect();
    calculateTextRect();

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

    QRectF rect;
    rect = rect.united(_symbolRect);
    rect = rect.united(_textRect);

    return rect.adjusted(-adj, -adj, adj, adj);
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
#warning ToDo: Implement me
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

    // Text pen
    QPen textPen;
    textPen.setStyle(Qt::SolidLine);
    textPen.setColor(Qt::black);

    // Text option
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Draw text
    painter->setPen(textPen);
    painter->setFont(QFont());
    QTransform t = painter->transform();
    switch (_textDirection) {
    case LeftToRight:
    case RightToLeft:
        t.rotate(0);
        break;

    case TopToBottom:
        t.translate(_textRect.bottomLeft().x()-_textRect.width()/2, _textRect.bottomLeft().y());
        t.rotate(-90);
        break;

    case BottomToTop:
        t.translate(_textRect.topLeft().x()+_textRect.width()/2, _textRect.topLeft().y());
        t.rotate(-90);
        break;
    }
    painter->setTransform(t);
    painter->drawText(_textRect, text(), textOption);
}

void Connector::calculateSymbolRect()
{
    _symbolRect = QRectF(-SIZE*_settings.gridSize/2.0, -SIZE*_settings.gridSize/2.0, SIZE*_settings.gridSize, SIZE*_settings.gridSize);
}

void Connector::calculateTextRect()
{
    if (text().isEmpty()) {

        _textRect = QRectF();
        _textDirection = LeftToRight;

    } else {

        // Get text width
        QFontMetrics fontMetrics((QFont()));
        int textWidth = fontMetrics.width(text());
        int textHeight = fontMetrics.height();

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

        // Update the text rectangle
        switch (_textDirection) {
        case LeftToRight:
            _textRect = QRectF(_symbolRect.topRight().x() + TEXT_PADDING, _symbolRect.topRight().y(), textWidth, textHeight);
            break;

        case RightToLeft:
            _textRect = QRectF(_symbolRect.topLeft().x() - textWidth - TEXT_PADDING, _symbolRect.topLeft().y(), textWidth, textHeight);
            break;

        case TopToBottom:
            _textRect = QRectF(_symbolRect.bottomLeft().x(), _symbolRect.bottomRight().y() + TEXT_PADDING, textWidth, textHeight);
            break;

        case BottomToTop:
            _textRect = QRectF(_symbolRect.bottomLeft().x(), _symbolRect.bottomRight().y() - textWidth - TEXT_PADDING , textWidth, textHeight);
            break;
        }

    }
}
