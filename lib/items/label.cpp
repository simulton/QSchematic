#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QJsonObject>
#include "label.h"
#include "../commands/commandlabelrename.h"
#include "../scene.h"

const QColor COLOR_LABEL             = QColor("#000000");
const QColor COLOR_LABEL_HIGHLIGHTED = QColor("#dc2479");
const qreal LABEL_TEXT_PADDING = 2;

using namespace QSchematic;

Label::Label(QGraphicsItem* parent) :
    Item(Item::LabelType, parent)
{
    setSnapToGrid(false);
}

QJsonObject Label::toJson() const
{
    QJsonObject object;

#warning ToDo: Add font
    object.insert("text", text());
    object.insert("connction point x", _connectionPoint.x());
    object.insert("connction point y", _connectionPoint.y());

    object.insert("item", Item::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool Label::fromJson(const QJsonObject& object)
{

    Item::fromJson(object["item"].toObject());

    setText(object["text"].toString());
    _connectionPoint.rx() = object["connection point x"].toInt();
    _connectionPoint.ry() = object["connection point y"].toInt();

    return true;
}

std::unique_ptr<Item> Label::deepCopy() const
{
    auto clone = std::make_unique<Label>(parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void Label::copyAttributes(Label& dest) const
{
    Item::copyAttributes(dest);

    dest._text = _text;
    dest._font = _font;
    dest._textRect = _textRect;
    dest._connectionPoint = _connectionPoint;
}

QRectF Label::boundingRect() const
{
    return _textRect;
}

void Label::setText(const QString& text)
{
    if (scene()) {
        scene()->undoStack()->push(new CommandLabelRename(this, text));
    } else {
        _text = text;
        calculateTextRect();
        emit textChanged(_text);
    }
}

void Label::setFont(const QFont& font)
{
    _font = font;

    calculateTextRect();
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
        painter->setPen(penLine);
        painter->setBrush(brushLine);
        painter->drawLine(_textRect.center(), mapFromParent(_connectionPoint));

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
