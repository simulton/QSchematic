#include "widget.h"
#include "../scene.h"

#include <QVBoxLayout>
#include <QSizeGrip>
#include <QGraphicsRectItem>
#include <QGraphicsProxyWidget>
#include <QPainter>

using namespace QSchematic;

Widget::Widget(int type, QGraphicsItem* parent) :
    Item(type, parent)
{
    m_proxy = new QGraphicsProxyWidget(this);

    setHighlightEnabled(false);
}

void
Widget::setWidget(QWidget* widget)
{
    if (!widget)
        return;

    m_widget = widget;
    m_widget->move(m_border_width, m_border_width);
    m_widget->installEventFilter(this);

    // Widget layout
    // Also add resize grip
    auto size_grip = new QSizeGrip(m_widget);
    auto layout = new QVBoxLayout(m_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(size_grip, 0, Qt::AlignRight | Qt::AlignBottom);

    // Proxy
    m_proxy->setWidget(m_widget);

    // Rectangle
    update_rect();
}

std::shared_ptr<Item>
Widget::deepCopy() const
{
    // Note: This is not copyable as we'd need a way to clone the underlying m_widget too (eg. via a user supplied
    //       factory or similar.

    return { };
}

QRectF
Widget::boundingRect() const
{
    return m_rect;
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

    painter->restore();
}

bool
Widget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_widget) {
        if (event->type() == QEvent::Resize) {
            update_rect();
            return true;
        }
        return false;
    }
    else
        return true;
        //return QSchematic::Item::eventFilter(obj, event);
}

void
Widget::update_rect()
{
    if (m_widget)
        m_rect = QRect(m_widget->geometry().adjusted(-m_border_width, -m_border_width, m_border_width, m_border_width));
}
