#include "widget.hpp"
#include "../scene.hpp"

#include <QGraphicsRectItem>
#include <QGraphicsProxyWidget>
#include <QPainter>

using namespace QSchematic::Items;

Widget::Widget(int type, QGraphicsItem* parent) :
    RectItem(type, parent)
{
    m_proxy = new QGraphicsProxyWidget(this);

    setHighlightEnabled(false);
    setAllowMouseResize(true);
    setAllowMouseRotate(false);

    setZValue(10);
}

void
Widget::setWidget(widgetFactory factory)
{
    // Bookkeeping
    m_factory = std::move(factory);

    // Nothing else to do if there's no factory
    if (!m_factory)
        return;

    // Create widget
    setWidget(m_factory());
}

void
Widget::setWidget(QWidget* widget)
{
    // Sanity check
    if (!widget) [[unlikely]]
        return;

    // Requirement according to QGraphicsProxyWidget documentation
    // Note: No need to delete the widget as it has a parent
    if (widget->parent())
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
    if (!m_factory)
        return { };

    // Create the clone
    auto clone = std::make_shared<Widget>(type(), parentItem());
    copyAttributes(*clone);

    // Set the widget
    clone->setWidget(m_factory);

    return clone;
}

void
Widget::copyAttributes(Widget& dest) const
{
    // Base class
    RectItem::copyAttributes(dest);

    // Attributes
    dest.m_factory = m_factory;
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
