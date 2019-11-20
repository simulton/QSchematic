#pragma once

#include <memory>
#include <QGraphicsObject>
#include <gpds/serialize.h>
#include "../types.h"
#include "../settings.h"

#include <QDebug>


#define TINKER__PTR_TRACKER__LOG_LEVEL 1
#define TINKER__MAKE_SH__USERSPACE_TRACKER true


template <typename ...T>
auto operator <<(QDebug debug, const std::weak_ptr<T...>& val) -> QDebug
{
    auto raw_ptr = val.lock().get();
    debug << "std::weak_ptr{ "
          << raw_ptr
          << ", use_count: "
          << val.use_count()
          << " }";
    return debug;
}

template <typename ...T>
auto operator <<(QDebug debug, const std::shared_ptr<T...>& val) -> QDebug
{
    debug << "std::shared_ptr{ "
          << val.get()
          << ", use_count: "
          << val.use_count()
          << " }";
    return debug;
}

inline
    auto operator <<(QDebug debug, const std::string& val) -> QDebug
{
    debug << val.c_str();
    return debug;
}




#include <QGraphicsScene>

namespace QSchematic {

    class Scene;
    class Item;




    // TODO: move to item-utils place
    template <typename T>
    inline auto dissociate_item(std::shared_ptr<T> item) -> void {
        dissociate_item(item.get());
    }

    inline auto dissociate_item(QGraphicsItem* item) -> void {
        item->setParentItem(nullptr);

        if ( auto scene = item->scene() ) {
            scene->removeItem(item);
        }
    }

    template <typename T, template <typename ValT> typename ContainerT>
    inline auto dissociate_items(ContainerT<std::shared_ptr<T>> items) -> void {
        for ( auto item : items ) {
            dissociate_item(item.get());
        }
    }





    namespace SharedPtrTracker {

    using PtrTrackerBaseT = Item;

    extern std::unordered_map<const PtrTrackerBaseT*, std::weak_ptr<PtrTrackerBaseT>>
        _global_items_shared_ptr_registry;

    extern std::vector<std::shared_ptr<PtrTrackerBaseT>>
        _global_eternalized_shared_ptr_registry;

    extern int _global_alloc_counter;


    inline
    auto _dbg_inspect_shptr_registry() -> void
    {
        auto _d = qDebug();
        _d << endl << "{";
        for (auto pair : _global_items_shared_ptr_registry ) {
            _d << "    " << pair << endl;
        }
        _d << "}" << endl;
    }

    template <typename InPtrT>
    auto obtain_weak_pointer(InPtrT* ptr) -> std::weak_ptr<PtrTrackerBaseT>
    {
        auto qg_ptr = static_cast<const PtrTrackerBaseT*>(ptr);
        if ( _global_items_shared_ptr_registry.find(qg_ptr) != end(_global_items_shared_ptr_registry) ) {
            auto w_ptr = _global_items_shared_ptr_registry.at(qg_ptr);
            if ( not w_ptr.expired() ) {
                #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
                    qDebug() << "SHPTR-REG => got ptr" << ptr << " => " << w_ptr;
                #endif
                return w_ptr;
            }
            else {
                #if TINKER__PTR_TRACKER__LOG_LEVEL >= 2
                    qDebug() << "SHPTR-REG => got ptr, but it's not alive anymore!" << ptr;
                #endif
                return {};
            }
        }
        else {
            #if TINKER__PTR_TRACKER__LOG_LEVEL >= 2
                qDebug() << "SHPTR-REG => no matching shared-ptr found!" << ptr;
            #endif
            return {};
        }
    }

    template <typename WantedT = PtrTrackerBaseT, typename InPtrT>
    auto obtain_shared_pointer(InPtrT* ptr) -> std::shared_ptr<WantedT>
    {
        auto w_ptr = obtain_weak_pointer(ptr);
        auto sh_ptr = std::dynamic_pointer_cast<WantedT>(w_ptr.lock());
        return sh_ptr;
    }

    template <typename InPtrT>
    auto assert_expired(InPtrT* ptr) -> bool
    {
        auto w_ptr = obtain_weak_pointer(ptr);
        return w_ptr.expired();
    }

    template <typename T>
    auto eternalize_pointer_life(std::shared_ptr<T> ptr) -> void {
        _global_eternalized_shared_ptr_registry.push_back(ptr);
    }

    template <typename InstanceT, typename ...ArgsT>
    auto mk_sh(ArgsT&& ...args) -> std::shared_ptr<InstanceT>
    {
        #if TINKER__MAKE_SH__USERSPACE_TRACKER == true
            #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
                auto count = ++_global_alloc_counter;
                qDebug() << "mk_sh<...>() -> (" << count << ")";
            #endif
            auto sh_ptr = std::make_shared<InstanceT>(args...);
            auto root_type_ptr = static_cast<PtrTrackerBaseT*>(sh_ptr.get());

            #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
                qDebug() << "mk_sh... (" << count << ")" << root_type_ptr << sh_ptr;
            #endif

            _global_items_shared_ptr_registry.insert({root_type_ptr, sh_ptr});

            #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
                qDebug() << "mk_sh<...>() -> / (" << count << ") registry.size = " << _global_items_shared_ptr_registry.size() << endl;
            #endif

            return sh_ptr;

        #else
            return std::make_shared<InstanceT>(std::forward(args)...);
        #endif
    }
    // TODO: explicit _maybe version, others will assert-throw if expectations unmet
    // TODO: cleanup_stale()
    // TODO: register_untracked_shptr(shptr/weakptr)
    // TODO: mk_sh<>() add capability of tracking TIMESTAMP, SERIAL and/or call-stack-point
    // TODO: assert_shptr_expired(shptr)
    // TODO: assert_shptr_alive(shptr)

    }




template <typename T, typename ...ArgsT>
auto mk_sh(ArgsT&&... args) -> std::shared_ptr<T>
{
    return SharedPtrTracker::mk_sh<T>(std::forward<ArgsT>(args)...);
}

template <typename WantedT, typename T>
auto adopt_origin_instance(std::shared_ptr<T> val) -> std::shared_ptr<WantedT>
{
    return std::dynamic_pointer_cast<WantedT>(val);
}

template <typename T>
auto adopt_origin_instance(std::shared_ptr<T> val) -> std::shared_ptr<T>
{
    return val;
}



    class Item : public QGraphicsObject, public gpds::serialize
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
            return SharedPtrTracker::obtain_shared_pointer<RetT>(this);
        }

        template <typename RetT = Item>
        auto sharedPtr() -> std::shared_ptr<RetT>
        {
            return SharedPtrTracker::obtain_shared_pointer<RetT>(this);
        }

        template <typename RetT = SharedPtrTracker::PtrTrackerBaseT>
        auto weakPtr() const -> std::weak_ptr<RetT>
        {
            return SharedPtrTracker::obtain_weak_pointer(this);
        }

        template <typename RetT = SharedPtrTracker::PtrTrackerBaseT>
        auto weakPtr() -> std::weak_ptr<RetT>
        {
            return SharedPtrTracker::obtain_weak_pointer(this);
        }
        /// @}





        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;
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

    signals:
        void moved(Item& item, const QVector2D& movedBy);
        void rotated(Item& item, const qreal rotation);
        void showPopup(const Item& item);
        void highlightChanged(const Item& item, bool isHighlighted);
        void settingsChanged();

    protected:
        Settings _settings;

        void copyAttributes(Item& dest) const;
        void addItemTypeIdToContainer(gpds::container& container) const;

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
