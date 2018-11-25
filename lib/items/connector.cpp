#include <QtMath>
#include <QPainter>
#include <QFontMetrics>
#include "connector.h"
#include "node.h"

const qreal SIZE               = 1;
const QColor COLOR_BODY_FILL   = QColor(Qt::green);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;
const int TEXT_PADDING         = 10;

using namespace QSchematic;

Connector::Connector(QGraphicsItem* parent) :
    Item(Item::ConnectorType, parent),
    _snapPolicy(NodeSizerectOutline),
    _textDiretion(Direction::LeftToRight)
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
        switch (_snapPolicy) {
        case Anywhere:
        {
            if (snapToGrid()) {
                return _settings.snapToGridPoint(proposedPos);
            } else {
                return proposedPos.toPoint();
            }
        }

        case NodeSizerect:
        {
            QPainterPath path;
            const Node* parentNode = qgraphicsitem_cast<const Node*>(parentItem());
            if (!parentNode) {
                return proposedPos;
            }
            return snapPointToRect(proposedPos, QRectF(0, 0, parentNode->size().width()*_settings.gridSize, parentNode->size().height()*_settings.gridSize));
        }

        case NodeSizerectOutline:
        {
            QPainterPath path;
            const Node* parentNode = qgraphicsitem_cast<const Node*>(parentItem());
            if (!parentNode) {
                return proposedPos;
            }
            return snapPointToRectOutline(proposedPos, QRectF(0, 0, parentNode->size().width()*_settings.gridSize, parentNode->size().height()*_settings.gridSize));
        }

        case NodeShape:
        {
            return snapPointToPath(proposedPos, QPainterPath());
        }
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
    switch (_textDiretion) {
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

QPointF Connector::snapPointToRect(QPointF point, const QRectF& rect) const
{
    // Clip X
    if (point.x() < rect.x()) {
        point.rx() = rect.x();
    } else if (point.x() > rect.width()) {
        point.rx() = rect.width();
    }

    // Clip Y
    if (point.y() < rect.y()) {
        point.ry() = rect.y();
    } else if (point.y() > rect.height()) {
        point.ry() = rect.height();
    }

    // Snap to grid
    return _settings.snapToGridPoint(point);
}

QPointF Connector::snapPointToRectOutline(const QPointF& point, const QRectF& rect) const
{
    // Create a list of points that we might snap to. Start topLeft and rotate CW. Don't add corners.
    QList<QPointF> listSnapPoints;
    unsigned nbrPoints;

    // Add points of top edge
    const QLineF topEdge(rect.topLeft(), rect.topRight());
    nbrPoints = topEdge.length() / _settings.gridSize;
    for (unsigned i = 1; i < nbrPoints; i++)
        listSnapPoints.append(QPointF(rect.topLeft().x() + i*_settings.gridSize, rect.topLeft().y()));

    // Add points of right edge
    const QLineF rightEdge(rect.topRight(), rect.bottomRight());
    nbrPoints = rightEdge.length() / _settings.gridSize;
    for (unsigned i = 1; i < nbrPoints; i++)
        listSnapPoints.append(QPointF(rect.topRight().x(), rect.topRight().y() + i*_settings.gridSize));

    // Add points of bottom edge
    const QLineF bottomEdge(rect.bottomRight(), rect.bottomLeft());
    nbrPoints = bottomEdge.length() / _settings.gridSize;
    for (unsigned i = 1; i < nbrPoints; i++)
        listSnapPoints.append(QPointF(rect.bottomLeft().x() + i*_settings.gridSize, rect.bottomLeft().y()));

    // Add points of left edge
    const QLineF leftEdge(rect.topLeft(), rect.bottomLeft());
    nbrPoints = leftEdge.length() / _settings.gridSize;
    for (unsigned i = 1; i < nbrPoints; i++)
        listSnapPoints.append(QPointF(rect.topLeft().x(), rect.topLeft().y() + i*_settings.gridSize));

    // We're done if there are no points in the list
    if (listSnapPoints.count() <= 0) {
        return point;
    }

    // Loop through list, find shortest distance.
    QPointF& nearestSnapPoint = listSnapPoints.first();
    for (auto& snapPoint : listSnapPoints) {
        QLineF currentDistanceLine(point.x(), point.y(), snapPoint.x(), snapPoint.y());
        QLineF shortestDistanceLine(point.x(), point.y(), nearestSnapPoint.x(), nearestSnapPoint.y());
        if (currentDistanceLine.length() < shortestDistanceLine.length()) {
            nearestSnapPoint = snapPoint;
        }
    }

    return nearestSnapPoint;
}

QPointF Connector::snapPointToPath(const QPointF& point, const QPainterPath& path) const
{
    Q_UNUSED(path);
#warning ToDo: Implement me

    return point;
}

void Connector::calculateSymbolRect()
{
    _symbolRect = QRectF(-SIZE*_settings.gridSize/2.0, -SIZE*_settings.gridSize/2.0, SIZE*_settings.gridSize, SIZE*_settings.gridSize);
}

void Connector::calculateTextRect()
{
    if (text().isEmpty()) {

        _textRect = QRectF();
        _textDiretion = LeftToRight;

    } else {

        // Get text width
        QFontMetrics fontMetrics((QFont()));
        int textWidth = fontMetrics.width(text());
        int textHeight = fontMetrics.height();

        // Figure out the text direction
        {
            QRectF fullRect;
            fullRect = fullRect.united(_symbolRect);
            fullRect = fullRect.united(_textRect);

            _textDiretion = LeftToRight;
            const Node* parentNode = qgraphicsitem_cast<const Node*>(parentItem());
            if (parentNode) {

                switch (_snapPolicy) {

                case Anywhere:
    #warning ToDo: Implement this
                    break;

                case NodeSizerect:
    #warning ToDo: Implement this
                    break;

                case NodeSizerectOutline:
                {
                    // Left
                    if (gridPointX() == 0) {
                        _textDiretion = LeftToRight;

                    // Right
                    } else if (gridPointX() == parentNode->size().width()) {
                        _textDiretion = RightToLeft;

                    // Top
                    } else if (gridPointY() == 0) {
                        _textDiretion = TopToBottom;

                    // Bottom
                    } else if (gridPointY() == parentNode->size().height()) {
                        _textDiretion = BottomToTop;
                    }
                }

                case NodeShape:
    #warning ToDo: Implement this
                    break;
                }

            }
        }

        // Update the text rectangle
        switch (_textDiretion) {
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
