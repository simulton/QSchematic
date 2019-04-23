#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include "label.h"
#include "../scene.h"

const QColor COLOR_LABEL             = QColor("#000000");
const QColor COLOR_LABEL_HIGHLIGHTED = QColor("#dc2479");
const qreal LABEL_TEXT_PADDING = 2;

using namespace QSchematic;

Label::Label(int type, QGraphicsItem* parent) :
    Item(type, parent),
    _hasConnectionPoint(true)
{
    setSnapToGrid(false);
}

Gds::Container Label::toContainer() const
{
    // Connection point
    Gds::Container connectionPoint;
    connectionPoint.addArgument("enabled", ( _hasConnectionPoint ? "true" : "false" ));
    connectionPoint.addEntry("x", _connectionPoint.x());
    connectionPoint.addEntry("y", _connectionPoint.y());

    // Root
    Gds::Container root;
    addItemTypeIdToContainer(root);
    root.addEntry("item", Item::toContainer());
    root.addEntry("text", text().toStdString());
    root.addEntry("connection_point", connectionPoint);

    return root;
}

void Label::fromContainer(const Gds::Container& container)
{
    Item::fromContainer( container.getEntry<Gds::Container>( "item" ) );
    setText( QString::fromStdString( container.getEntry<std::string>( "text" ) ) );

    // Connection point
    {
        Gds::Container connectionPointContainer = container.getEntry<Gds::Container>( "connection_point" );
#warning ToDo: Use argument
        _hasConnectionPoint = false;
        _connectionPoint.setX( connectionPointContainer.getEntry<double>( "x" ) );
        _connectionPoint.setY( connectionPointContainer.getEntry<double>( "y" ) );
    }
}

std::unique_ptr<Item> Label::deepCopy() const
{
    auto clone = std::make_unique<Label>(type(), parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void Label::copyAttributes(Label& dest) const
{
    // Base class
    Item::copyAttributes(dest);

    // Attributes
    dest._text = _text;
    dest._font = _font;
    dest._textRect = _textRect;
    dest._hasConnectionPoint = _hasConnectionPoint;
    dest._connectionPoint = _connectionPoint;
}

QRectF Label::boundingRect() const
{
    return _textRect;
}

void Label::setText(const QString& text)
{
    _text = text;
    calculateTextRect();
    emit textChanged(_text);
}

void Label::setFont(const QFont& font)
{
    _font = font;

    calculateTextRect();
}

void Label::setHasConnectionPoint(bool enabled)
{
    _hasConnectionPoint = enabled;
}

bool Label::hasConnectionPoint() const
{
    return _hasConnectionPoint;
}

void Label::setConnectionPoint(const QPointF& connectionPoint)
{
    _connectionPoint = connectionPoint;

    Item::update();
}

void Label::calculateTextRect()
{
    QFontMetricsF fontMetrics(_font);
    _textRect = fontMetrics.boundingRect(_text);
    _textRect.adjust(-LABEL_TEXT_PADDING, -LABEL_TEXT_PADDING, LABEL_TEXT_PADDING, LABEL_TEXT_PADDING);
}

QString Label::text() const
{
    return _text;
}

QFont Label::font() const
{
    return _font;
}

QRectF Label::textRect() const
{
    return _textRect;
}

void Label::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Draw a dashed line to the wire if selected
    if (isHighlighted()) {
        // Line pen
        QPen penLine;
        penLine.setColor(COLOR_LABEL_HIGHLIGHTED);
        penLine.setStyle(Qt::DashLine);

        // Line brush
        QBrush brushLine;
        brushLine.setStyle(Qt::NoBrush);

        // Draw the connection line
        if (_hasConnectionPoint) {
            painter->setPen(penLine);
            painter->setBrush(brushLine);
            painter->drawLine(_textRect.center(), mapFromParent(_connectionPoint));
        }

        // Clear the text rectangle
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(_textRect.adjusted(penLine.width()/2, penLine.width()/2, -penLine.width()/2, -penLine.width()/2));

        // Draw the border around the label text
        painter->setPen(penLine);
        painter->setBrush(brushLine);
        painter->drawRect(_textRect);
    }

    // Text pen
    QPen textPen;
    textPen.setStyle(Qt::SolidLine);
    textPen.setColor(Qt::black);

    // Text option
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    textOption.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Draw the text
    painter->setPen(COLOR_LABEL);
    painter->setBrush(Qt::NoBrush);
    painter->setFont(_font);
    painter->drawText(_textRect, _text, textOption);
}
