#pragma once

#include "rectitem.hpp"

#include <functional>

class QGraphicsProxyWidget;

namespace QSchematic::Items
{

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
        using widgetFactory = std::function<QWidget*()>;

        explicit
        Widget(int type, QGraphicsItem* parent = nullptr);
        ~Widget() override = default;

        std::shared_ptr<Item>
        deepCopy() const override;

        /**
         * Set a widget.
         *
         * @details The factory pattern is necessary to allow an instance of this Widget item to be copied
         *          and saved/loaded to/from a file.
         */
        void
        setWidget(widgetFactory factory);

        /**
         * @note Using this function will result in a Widget item which cannot be copied or saved/loaded.
         *       It is strongly recommended to use the setWidget() overload which accepts a factory instead.
         */
        void
        setWidget(QWidget* widget);

        QRectF
        boundingRect() const override;

        void
        sizeChangedEvent(QSizeF oldSize, QSizeF newSize) override;

        void
        paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        int m_border_width    = 15;
        QPen m_border_pen     = QPen(Qt::NoPen);
        QBrush m_border_brush = QBrush(Qt::gray);

        void
        copyAttributes(Widget& dest) const;

    private:
        QRect m_rect;
        QGraphicsProxyWidget* m_proxy = nullptr;
        widgetFactory m_factory;

        void
        update_rect();
    };

}
