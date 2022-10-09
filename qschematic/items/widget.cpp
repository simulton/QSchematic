#include "widget.h"
#include "../scene.h"

#include <QGraphicsRectItem>
#include <QGraphicsProxyWidget>
#include <QPainter>

using namespace QSchematic;

Widget::Widget(int type, QGraphicsItem* parent) :
    RectItem(type, parent)
{
    m_proxy = new QGraphicsProxyWidget(this);

    setHighlightEnabled(false);
    setAllowMouseResize(true);
    setAllowMouseRotate(false);
}

void
Widget::setWidget(QWidget* widget)
{
    if (!widget)
        return;
    if (widget->parent())    // Requirement according to QGraphicsProxyWidget documentation
        return;

    // Widget
    widget->move(m_border_width, m_border_width);

    // Proxy
    m_proxy->setWidget(widget);

    // Resize ourselves
    setSize(widget->size());

    // Rectangle
    update_rect();
}

std::shared_ptr<Item>
Widget::deepCopy() const
{
    // Note: This is not copyable as we'd need a way to clone the underlying widget too (eg. via a user supplied
    //       factory or similar.

    return { };
}

QRectF
Widget::boundingRect() const
{
    return m_rect;
}

void
Widget::sizeChangedEvent(QSizeF oldSize, QSizeF newSize)
{
    update_rect();

    if (auto widget = m_proxy->widget(); widget)
        widget->setGeometry(m_rect.adjusted(m_border_width, m_border_width, -m_border_width, -m_border_width));
}

void
Widget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    // Draw the bounding rect if debug mode is enabled
    if (_settings.debug) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Qt::red));
        painter->drawRect(boundingRect());
    }

    // Border
    painter->setPen(m_border_pen);
    painter->setBrush(m_border_brush);
    painter->drawRect(m_rect);

    // Resize handles
    if (isSelected() && allowMouseResize())
        paintResizeHandles(*painter);

    // Rotate handle
    if (isSelected() && allowMouseRotate())
        paintRotateHandle(*painter);

    painter->restore();
}

void
Widget::update_rect()
{
    m_rect = sizeRect().adjusted(-m_border_width, -m_border_width, m_border_width, m_border_width).toRect();
}
