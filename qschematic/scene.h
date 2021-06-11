#pragma once

#include <algorithm>
#include <memory>
#include <functional>
#include <QGraphicsScene>
#include <QUndoStack>
#ifdef USE_GPDS
#include <gpds/serialize.hpp>
#endif
#include "wire_system/manager.h"
#include "settings.h"
#include "items/item.h"
#include "items/wire.h"
#include "qschematic_export.h"
//#include "utils/itemscustodian.h"

namespace QSchematic {

    class Node;
    class Connector;
    class WireNet;

    class QSCHEMATIC_EXPORT Scene :
        public QGraphicsScene
#ifdef USE_GPDS
        ,public gpds::serialize
#endif
    {
        Q_OBJECT
        Q_DISABLE_COPY(Scene)

    public:
        enum Mode {
            NormalMode,
            WireMode,

            UserMode = 1023
        };
        Q_ENUM(Mode)

        explicit Scene(QObject* parent = nullptr);
        virtual ~Scene() override = default;

#ifdef USE_GPDS
        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;
#endif

        void setSettings(const Settings& settings);
        void setWireFactory(const std::function<std::shared_ptr<Wire>()>& factory);
        void setMode(int mode);
        int mode() const;
        void toggleWirePosture();

        bool isDirty() const;
        void clearIsDirty();

        void clear();
        bool addItem(const std::shared_ptr<Item>& item);
        bool removeItem(const std::shared_ptr<Item> item);
        QList<std::shared_ptr<Item>> items() const;
        QList<std::shared_ptr<Item>> items(int itemType) const;

        /**
         * Get list of items of a certain type.
         *
         * @tparam T The type of item.
         * @return List of all items of type `T`.
         */
        template<typename T>
        [[nodiscard]]
        std::vector<std::shared_ptr<T>> items() const
        {
            const auto& itms = items();

            std::vector<std::shared_ptr<T>> ret;
            ret.reserve(itms.size());

            for (const auto& item : itms)
                if (auto casted = std::dynamic_pointer_cast<T>(item); casted)
                    ret.emplace_back(std::move(casted));

            return ret;
        }

        QList<std::shared_ptr<Item>> itemsAt(const QPointF& scenePos, Qt::SortOrder order = Qt::DescendingOrder) const;
        std::vector<std::shared_ptr<Item>> selectedItems() const;
        std::vector<std::shared_ptr<Item>> selectedTopLevelItems() const;
        QList<std::shared_ptr<Node>> nodes() const;
        [[nodiscard]] std::shared_ptr<Node> nodeFromConnector(const QSchematic::Connector& connector) const;
        QList<QPointF> connectionPoints() const;
        QList<std::shared_ptr<Connector>> connectors() const;
        std::shared_ptr<wire_system::manager> wire_manager() const;
        void itemHoverEnter(const std::shared_ptr<const Item>& item);
        void itemHoverLeave(const std::shared_ptr<const Item>& item);
        void removeLastWirePoint();
        void removeUnconnectedWires();
        bool addWire(const std::shared_ptr<Wire>& wire);
        bool removeWire(const std::shared_ptr<Wire>& wire);
        QList<std::shared_ptr<WireNet>> nets(const std::shared_ptr<net> wireNet) const;

        void undo();
        void redo();
        QUndoStack* undoStack() const;

    signals:
        void modeChanged(int newMode);
        void isDirtyChanged(bool isDirty);
        void itemAdded(const std::shared_ptr<const Item> item);
        void itemRemoved(const std::shared_ptr<const Item> item);
        void itemHighlighted(const std::shared_ptr<const Item>& item);

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
        virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;
        virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

        /* This gets called just before the item is actually being moved by moveBy. Subclasses may
         * implement this to implement snapping to elements other than the grid
         */
        virtual QVector2D itemsMoveSnap(const std::shared_ptr<Item>& item, const QVector2D& moveBy) const;

    private:
        void renderCachedBackground();
        void setupNewItem(Item& item);
        std::shared_ptr<Item> sharedItemPointer(const Item& item) const;
        void generateConnections();
        void finishCurrentWire();

        // TODO add to "central" sh-ptr management
        QList<std::shared_ptr<Item>> _keep_alive_an_event_loop;
        /**
         * Used to store a list of "Top-Level" items. These are the only items
         * moved by the scene. Scene::addItem automatically adds the items to
         * this list. Items that are children of another Item should
         * not be in the list.
         */
        QList<std::shared_ptr<Item>> _items;

        // Note: haven't investigated destructor specification, but it seems
        // this can be skipped, although it would be: explicit, more efficient,
        // and possibly required in more complex destruction scenarios — but
        // we're skipping that extra work now / ozra
        //
        // ItemUtils::ItemsCustodian<Item> _items;
        // ItemUtils::ItemsCustodian<WireNet> m_nets;

        Settings _settings;
        QPixmap _backgroundPixmap;
        std::function<std::shared_ptr<Wire>()> _wireFactory;
        int _mode;
        std::shared_ptr<Wire> _newWire;
        bool _newWireSegment;
        bool _invertWirePosture;
        bool _movingNodes;
        QPointF _lastMousePos;
        QMap<std::shared_ptr<Item>, QPointF> _initialItemPositions;
        QPointF _initialCursorPosition;
        QUndoStack* _undoStack;
        std::shared_ptr<wire_system::manager> m_wire_manager;
        Item* _highlightedItem;

    private slots:
        void updateNodeConnections(const Node* node) const;
        void wirePointMoved(wire& rawWire, int index);
    };

}
