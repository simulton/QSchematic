#pragma once

#include "settings.hpp"
#include "items/item.hpp"   // For QGraphicsItem::type() overload

#include <QBrush>
#include <QPen>
#include <QGraphicsRectItem>

namespace QSchematic
{

    /**
     * Scene background item.
     *
     * @note We are intentionally not deriving from QSchematic::Items::Item as we don't want this to
     *       participate in any user facing logic. However, we still have to return a proper type for the
     *       QGraphicsItem::type() overload for qgraphicsitem_cast<> to work properly.
     */
    class Background :
        public QGraphicsRectItem
    {
    public:
        explicit
        Background(QGraphicsItem* parent = nullptr);

        ~Background() override = default;

        void
        setSettings(const Settings& settings);

        [[nodiscard]]
        int
        type() const override
        {
            return QSchematic::Items::Item::ItemType::BackgroundType;
        }

        void
        paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        QPen m_background_pen;
        QBrush m_background_brush;
        QPen m_grid_pen;
        QBrush m_grid_brush;

        [[nodiscard]]
        Settings
        settings() const noexcept
        {
            return m_settings;
        }

    private:
        Settings m_settings;
    };

}
