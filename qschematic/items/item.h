#pragma once

#include <memory>
#include <QGraphicsObject>
#include <gpds/serialize.h>
#include "../types.h"
#include "../settings.h"

namespace QSchematic {

    class Scene;

//#define UNIQUE_ORIGIN_T
#define SHARED_ORIGIN_T

#ifdef UNIQUE_ORIGIN_T
    template <typename T>
    using OriginMgrT = std::unique_ptr<T>;

    template <typename T, typename ...ArgsT>
    auto make_origin(ArgsT&&... args) -> OriginMgrT<T>
    {
        return std::make_unique<T>(std::forward<ArgsT>(args)...);
    }

    template <typename WantedT, typename T>
    auto adopt_origin_instance(OriginMgrT<T> val) -> WantedT*
    {
        return dynamic_cast<WantedT*>(val.release());
    }

    template <typename T>
    auto adopt_origin_instance(OriginMgrT<T> val) -> T*
    {
        return val.release();
    }

#else
    template <typename T>
    using OriginMgrT = std::shared_ptr<T>;

    template <typename T, typename ...ArgsT>
    auto make_origin(ArgsT&&... args) -> OriginMgrT<T>
    {
        return std::make_shared<T>(std::forward<ArgsT>(args)...);
    }

    template <typename WantedT, typename T>
    auto adopt_origin_instance(OriginMgrT<T> val) -> OriginMgrT<WantedT>
    {
        return std::dynamic_pointer_cast<WantedT>(val);
    }

    template <typename T>
    auto adopt_origin_instance(OriginMgrT<T> val) -> OriginMgrT<T>
    {
        return val;
    }

#endif

    class Item : public QGraphicsObject, public Gpds::Serialize
    {
        friend class CommandItemSetVisible;

        Q_OBJECT
        Q_DISABLE_COPY(Item)

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

        const QString JSON_ID_STRING = QStringLiteral("type_id");

        Item(int type, QGraphicsItem* parent = nullptr);
        virtual ~Item() override;

        virtual Gpds::Container toContainer() const override;
        virtual void fromContainer(const Gpds::Container& container) override;
        virtual OriginMgrT<Item> deepCopy() const = 0;

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

    signals:
        void moved(Item& item, const QVector2D& movedBy);
        void rotated(Item& item, const qreal rotation);
        void showPopup(const Item& item);
        void highlightChanged(const Item& item, bool isHighlighted);
        void settingsChanged();

    protected:
        Settings _settings;

        void copyAttributes(Item& dest) const;
        void addItemTypeIdToContainer(Gpds::Container& container) const;

        Scene* scene() const;

        bool isHighlighted() const;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    private slots:
        void posChanged();
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
