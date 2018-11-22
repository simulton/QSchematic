#pragma once

#include <QGraphicsObject>
#include "../types.h"
#include "../settings.h"

namespace QSchematic {

    class Scene;

    class Item : public QGraphicsObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(Item)

    public:
        enum ItemType {
            NodeType      = QGraphicsItem::UserType + 1,
            WireType,
            ConnectorType,
            LabelType,
        };

        Item(ItemType type, QGraphicsItem* parent = nullptr);
        virtual ~Item() override = default;

        int type() const final;
        void setGridPoint(const QPoint& newGridPoint);
        void setGridPoint(int x, int y);
        void setGridPointX(int x);
        void setGridPointY(int y);
        QPoint gridPoint() const;
        int gridPointX() const;
        int gridPointY() const;
        void setSettings(const Settings& settings);
        const Settings& settings() const;
        void setMovable(bool enabled);
        bool movable() const;
        void setSnapToGrid(bool enabled);
        bool snapToGrid() const;
        void setHighlighted(bool highlighted);
        void setHighlightEnabled(bool enabled);
        QPixmap toPixmap(qreal scale = 1.0);
        virtual void update();
        virtual QWidget* popupInfobox() const;

    signals:
        void moved(Item& item, const QVector2D& movedBy);
        void showPopup(const Item& item);
        void highlightChanged(const Item& item, bool highlighted);

    protected:
        Settings _settings;

        bool highlighted() const;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    private slots:
        void timerTimeout();
        void posChanged();

    private:
        ItemType _type;
        bool _snapToGrid;
        bool _highlightEnabled;
        bool _highlighted;
        QPoint _oldGridPoint;
        QTimer* _hoverTimer;
    };

}
