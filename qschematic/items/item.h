#pragma once

#include "itemfunctions.h"
#include "../types.h"
#include "../settings.h"

#include <gpds/serialize.hpp>
#include <QDebug>
#include <QGraphicsObject>
#include <QWidget>

#include <memory>

namespace QSchematic
{
    class Scene;

    class Item :
        public QGraphicsObject,
        public gpds::serialize,
        public std::enable_shared_from_this<Item>
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Item)

    public:
        enum ItemType {
            NodeType               = QGraphicsItem::UserType + 1,
            WireType,
            WireRoundedCornersType,
            ConnectorType,
            LabelType,
            SplineWireType,

            QSchematicItemUserType = QGraphicsItem::UserType + 100
        };
        Q_ENUM(ItemType)

        Item(int type, QGraphicsItem* parent = nullptr);
        ~Item() override;

        /**
         * These funcs should be the only source for obtaining a canonical
         * shared-/weak-ptr to the item. It _must_ be allocated with make_shared
         * or shared-constructor — ,no compile time check validates that.
         * For convenience it's also possible to cast by simply explicitly
         * passing a template arg
         */
        /// @{
        template <typename RetT = Item>
        auto sharedPtr() const -> std::shared_ptr<const RetT>
        {
            if constexpr (std::is_same_v<RetT, Item>)
                return shared_from_this();
            else
                return std::dynamic_pointer_cast<const RetT>(shared_from_this());
        }

        template <typename RetT = Item>
        auto sharedPtr() -> std::shared_ptr<RetT>
        {
            if constexpr (std::is_same_v<RetT, Item>)
                return shared_from_this();
            else
                return std::dynamic_pointer_cast<RetT>(shared_from_this());
        }

        template <typename RetT = Item>
        auto weakPtr() const -> std::weak_ptr<const RetT>
        {
            if constexpr (std::is_same_v<RetT, Item>)
                return weak_from_this();
            else
                return std::dynamic_pointer_cast<const RetT>(weak_from_this());
        }

        template <typename RetT = Item>
        auto weakPtr() -> std::weak_ptr<RetT>
        {
            if constexpr (std::is_same_v<RetT, Item>)
                return weak_from_this();
            else
                return std::dynamic_pointer_cast<RetT>(weak_from_this());
        }
        /// @}

        gpds::container to_container() const override;
        void from_container(const gpds::container& container) override;
        virtual std::shared_ptr<Item> deepCopy() const = 0;

        int type() const final;
        void setGridPos(const QPoint& gridPos);
        void setGridPos(int x, int y);
        void setGridPosX(int x);
        void setGridPosY(int y);
        QPoint gridPos() const;
        int gridPosX() const;
        int gridPosY() const;
        void setPos(const QPointF& pos);
        void setPos(qreal x, qreal y);
        void setPosX(qreal x);
        void setPosY(qreal y);
        QPointF pos() const;
        qreal posX() const;
        qreal posY() const;
        void setScenePos(const QPointF& point);
        void setScenePos(qreal x, qreal y);
        void setScenePosX(qreal x);
        void setScenePosY(qreal y);
        QPointF scenePos() const;
        qreal scenePosX() const;
        qreal scenePosY() const;
        void moveBy(const QVector2D& moveBy);
        void setSettings(const Settings& settings);
        const Settings& settings() const;
        void setMovable(bool enabled);
        bool isMovable() const;
        void setSnapToGrid(bool enabled);
        bool snapToGrid() const;
        void setHighlighted(bool isHighlighted);
        void setHighlightEnabled(bool enabled);
        bool highlightEnabled() const;
        QPixmap toPixmap(QPointF& hotSpot, qreal scale = 1.0);
        virtual void update();
        Scene* scene() const;
        virtual std::unique_ptr<QWidget> popup() const { return nullptr; }

    signals:
        void moved(Item& item, const QVector2D& movedBy);
        void movedInScene(Item& item);
        void rotated(Item& item, const qreal rotation);
        void highlightChanged(const Item& item, bool isHighlighted);
        void settingsChanged();

    protected:
        Settings _settings;

        void copyAttributes(Item& dest) const;
        void addItemTypeIdToContainer(gpds::container& container) const;

        bool isHighlighted() const;
        QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    private slots:
        void posChanged();
        void scenePosChanged();
        void rotChanged();

    private:
        int _type;
        bool _snapToGrid;
        bool _highlightEnabled;
        bool _highlighted;
        QPointF _oldPos;
        qreal _oldRot;
    };

}
