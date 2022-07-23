#include "label.h"
#include "../scene.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QBrush>

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

gpds::container Label::to_container() const
{
    // Connection point
    gpds::container connectionPoint;
    connectionPoint.add_attribute("enabled", ( _hasConnectionPoint ? "true" : "false" ));
    connectionPoint.add_value("x", _connectionPoint.x());
    connectionPoint.add_value("y", _connectionPoint.y());

    // Root
    gpds::container root;
    addItemTypeIdToContainer(root);
    root.add_value("item", Item::to_container());
    root.add_value("text", text().toStdString());
    root.add_value("connection_point", connectionPoint);

    return root;
}

void Label::from_container(const gpds::container& container)
{
    Item::from_container(*container.get_value<gpds::container*>("item").value());
    setText(QString::fromStdString(container.get_value<std::string>("text").value_or("")));

    // Connection point
    const gpds::container* connectionPointContainer = container.get_value<gpds::container*>("connection_point").value_or(nullptr);
    if (connectionPointContainer) {
        auto attributeString = connectionPointContainer->get_attribute<std::string>( "enabled" );
        if ( attributeString.has_value() ) {
            _hasConnectionPoint = ( attributeString.value() == "true" );
        }
        _connectionPoint.setX(connectionPointContainer->get_value<double>("x").value_or(0));
        _connectionPoint.setY(connectionPointContainer->get_value<double>("y").value_or(0));
    }
}

std::shared_ptr<Item> Label::deepCopy() const
{
    auto clone = std::make_shared<Label>(type(), parentItem());
    copyAttributes(*clone);

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
    QRectF rect = _textRect;
    if(isHighlighted()) {
        rect = rect.united(QRectF(_textRect.center(), mapFromParent(_connectionPoint)));
    }
    return rect;
}

QPainterPath Label::shape() const
{
    QPainterPath path;
    path.addRect(_textRect);
    return path;
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

    // Draw the bounding rect if debug mode is enabled
    if (_settings.debug) {
        painter->setPen(Qt::red);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
        painter->setPen(Qt::blue);
        painter->drawPath(shape());
    }
}

void Label::mouseDoubleClickEvent([[maybe_unused]] QGraphicsSceneMouseEvent* event)
{
    emit doubleClicked();
}
