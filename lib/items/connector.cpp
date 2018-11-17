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
    _text("Foo"),
    _textDiretion(Direction::LeftToRight)
{
    // Flags
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    // Make sure that we are above the parent
    if (parentItem()) {
        setZValue(parentItem()->zValue() + 1);
    }

    calculateSymbolRect();
    calculateTextRect();
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

QPoint Connector::connectionPoint() const
{
    return _settings.toGridPoint(mapToScene(_settings.toScenePoint(QPoint(0, 0))));
}

QRectF Connector::boundingRect() const
{
    qreal adj = qCeil(PEN_WIDTH / 2.0) + _settings.highlightRectPadding;

    QRectF rect;
    rect = rect.united(_symbolRect);
    rect = rect.united(_textRect);

    return rect.adjusted(-adj, -adj, adj, adj);
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
    painter->drawText(_textRect, text(), textOption);
}

QPointF Connector::snapPointToPath(const QPointF& point, const QPainterPath& path) const
{
    Q_UNUSED(path);
#warning ToDo: Implement me

    return point.toPoint();
}

void Connector::calculateSymbolRect()
{
    _symbolRect = QRectF(-SIZE*_settings.gridSize/2.0, -SIZE*_settings.gridSize/2.0, SIZE*_settings.gridSize, SIZE*_settings.gridSize);
}

void Connector::calculateTextRect()
{
    // Update the text position stuff
    {
        QRectF symbolRect = _symbolRect;
        QFontMetrics fontMetrics((QFont()));
        int textWidth = fontMetrics.width(text());

        switch (_textDiretion) {
        case LeftToRight:
            _textRect = QRectF(symbolRect.topRight().x() + TEXT_PADDING, symbolRect.topRight().y(), textWidth, symbolRect.height());
            break;

        case RightToLeft:
            _textRect = QRectF(symbolRect.topLeft().x() - textWidth - TEXT_PADDING, symbolRect.topLeft().y(), textWidth, symbolRect.height());
            break;

        case TopToBottom:
            _textRect = QRectF(symbolRect.topLeft().x() - textWidth - TEXT_PADDING, symbolRect.topLeft().y(), textWidth, symbolRect.height());
            break;

        case BottomToTop:
            _textRect = QRectF(symbolRect.topLeft().x() - textWidth - TEXT_PADDING, symbolRect.topLeft().y(), textWidth, symbolRect.height());
            break;
        }
    }
}
