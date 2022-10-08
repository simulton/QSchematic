#pragma once

#include "item.h"

class QGraphicsRectItem;
class QGraphicsProxyWidget;

namespace QSchematic
{
    class Scene;

    class Widget :
        public Item
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Widget)

    public:
        Widget(int type, QWidget* widget, QGraphicsItem* parent = nullptr);
        ~Widget() override = default;

        std::shared_ptr<Item>
        deepCopy() const override;

        QRectF
        boundingRect() const override;

        void
        paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        bool
        eventFilter(QObject* obj, QEvent* event) override;

    private:
        QRect m_rect;
        QWidget* m_widget = nullptr;
        QGraphicsRectItem* m_handle = nullptr;
        QGraphicsProxyWidget* m_proxy = nullptr;

        void
        update_rect();
    };

}
