#pragma once

#include "settings.hpp"

#include <QBrush>
#include <QPen>
#include <QGraphicsRectItem>

namespace QSchematic
{

    class Background :
        public QGraphicsRectItem
    {
    public:
        explicit
        Background(QGraphicsItem* parent = nullptr);

        ~Background() override = default;

        void
        setSettings(const Settings& settings);

        void
        paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        QPen m_background_pen;
        QBrush m_background_brush;
        QPen m_grid_pen;
        QBrush m_grid_brush;

    private:
        Settings m_settings;
    };

}
