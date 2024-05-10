#pragma once

#include "settings.hpp"

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

    private:
        Settings m_settings;
    };

}
