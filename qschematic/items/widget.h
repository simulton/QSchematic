#pragma once

#include "rectitem.h"

class QGraphicsProxyWidget;

namespace QSchematic
{
    class Scene;

    /**
     * QWidget wrapper.
     *
     * @details This class allows encapsulating a QWidget in the scene.
     */
    class Widget :
        public RectItem
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
        sizeChangedEvent(QSizeF oldSize, QSizeF newSize) override;

        void
        paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    protected:
        int m_border_width    = 15;
        QPen m_border_pen     = QPen(Qt::NoPen);
        QBrush m_border_brush = QBrush(Qt::gray);

    private:
        QRect m_rect;
        QGraphicsProxyWidget* m_proxy = nullptr;

        void
        update_rect();
    };

}
