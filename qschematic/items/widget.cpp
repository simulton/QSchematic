#include "widget.h"
#include "../scene.h"

#include <QVBoxLayout>
#include <QSizeGrip>
#include <QGraphicsRectItem>
#include <QGraphicsProxyWidget>
#include <QPainter>

using namespace QSchematic;

static constexpr int BORDER = 10;

Widget::Widget(int type, QWidget* widget, QGraphicsItem* parent) :
    Item(type, parent)
{
    // Widget
    m_widget = widget;
    m_widget->move(BORDER, BORDER);
    m_widget->installEventFilter(this);

    // Proxy
    m_proxy = new QGraphicsProxyWidget(this);
    m_proxy->setWidget(m_widget);

    auto sizeGrip = new QSizeGrip(m_widget);
    auto layout = new QVBoxLayout(m_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(sizeGrip, 0, Qt::AlignRight | Qt::AlignBottom);

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
    painter->save();

    // Border
    painter->setPen(QPen(Qt::transparent));
    painter->setBrush(Qt::gray);
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
        return QSchematic::Item::eventFilter(obj, event);
}

void
Widget::update_rect()
{
    m_rect = QRect(m_widget->geometry().adjusted(-BORDER, -BORDER, BORDER, BORDER));
}
