#pragma once

#include "item.h"

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
        Widget(int type, QGraphicsItem* parent = nullptr);
        ~Widget() override = default;

        std::shared_ptr<Item>
        deepCopy() const override;

        void
        setWidget(QWidget* widget);

        QRectF
        boundingRect() const override;

        void
        paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        bool
        eventFilter(QObject* obj, QEvent* event) override;

    protected:
        int m_border_width    = 10;
        QPen m_border_pen     = QPen(Qt::NoPen);
        QBrush m_border_brush = QBrush(Qt::gray);

    private:
        QRect m_rect;
        QWidget* m_widget = nullptr;
        QGraphicsProxyWidget* m_proxy = nullptr;

        void
        update_rect();
    };

}
