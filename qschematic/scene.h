#pragma once

#include "settings.h"
#include "items/item.h"
#include "items/wire.h"
#include "wire_system/manager.h"
//#include "utils/itemscustodian.h"

#include <gpds/serialize.hpp>
#include <QGraphicsScene>
#include <QUndoStack>

#include <algorithm>
#include <memory>
#include <functional>

namespace QSchematic
{

    class Node;
    class Connector;
    class WireNet;

    class Scene :
        public QGraphicsScene,
        public gpds::serialize
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Scene)

    public:
        enum Mode {
            NormalMode,
            WireMode,

            UserMode = 1023
        };
        Q_ENUM(Mode)

        explicit Scene(QObject* parent = nullptr);
        ~Scene() override = default;

        gpds::container to_container() const override;
        void from_container(const gpds::container& container) override;

        void setSettings(const Settings& settings);
        void setWireFactory(const std::function<std::shared_ptr<Wire>()>& factory);
        void setMode(int mode);
        int mode() const;
        void toggleWirePosture();

        bool isDirty() const;
        void clearIsDirty();

        /**
         * Clears the scene.
         *
         * @note Consuming applications should always use this instead of the base class QGraphicsScene::clear() as we
         * have to update internal bookkeeping.
         */
        void
        clear();

        /**
         * Adds an item to the scene.
         *
         * @note This does not generate an undo/redo command. Consuming applications that want to add items should generally
         *       create an instance of `CommandItemAdd` and push that to the scene's command stack.
         *
         * @note Only top-level items should be added via this function. Child items should be added via the underlying
         *       parent/child relation ship (eg. `QGraphicsItem::setParentItem()`).
         *
         * @param item The item to add.
         * @return Success indicator.
         */
        bool
        addItem(const std::shared_ptr<Item>& item);

        /**
         * Removes an item from the scene.
         *
         * @note This does not generate an undo/redo command. Consuming applications that want to remove items should generally
         *       create an instance of `CommandItemRemove` and push that to the scene's command stack.
         *
         * @param item The item to remove.
         *
         * @return Success indicator.
         */
        bool
        removeItem(const std::shared_ptr<Item> item);

        /**
         * Get a list of all top-level items.
         *
         * @return The list of top-level items.
         */
        [[nodiscard]]
        QList<std::shared_ptr<Item>>
        items() const;

        /**
         * Get a list of all top-level items of a specified type.
         *
         * @param itemType The item type.
         * @return The list of top-level items.
         */
        [[nodiscard]]
        QList<std::shared_ptr<Item>>
        items(int itemType) const;

        /**
         * Get list of items of a certain type.
         *
         * @tparam T The type of item.
         * @return List of all items of type `T`.
         */
        template<typename T>
        [[nodiscard]]
        std::vector<std::shared_ptr<T>>
        items() const
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

        /**
         * Undo to last command.
         */
        void
        undo();

        /**
         * Redo the last command.
         */
        void
        redo();

        /**
         * Get the command (undo/redo) stack.
         *
         * @note The scene guarantees that this is a valid pointer as long as the scene itself is valid (alive).
         *
         * @return The command stack.
         */
        [[nodiscard]]
        QUndoStack*
        undoStack() const;

    signals:
        void modeChanged(int newMode);
        void isDirtyChanged(bool isDirty);
        void itemAdded(std::shared_ptr<Item> item);
        void itemRemoved(std::shared_ptr<Item> item);
        void itemHighlighted(const std::shared_ptr<const Item>& item);

    protected:
        Settings _settings;

        // QGraphicsScene
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
        void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
        void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
        void dropEvent(QGraphicsSceneDragDropEvent* event) override;
        void drawBackground(QPainter* painter, const QRectF& rect) override;

        /**
         * This gets called just before the item is actually being moved by moveBy. Subclasses may
         * implement this to implement snapping to elements other than the grid
         */
        [[nodiscard]]
        virtual
        QVector2D
        itemsMoveSnap(const std::shared_ptr<Item>& item, const QVector2D& moveBy) const;

        /**
         * Renders the background.
         *
         * @param rect The scene rectangle. This is guaranteed to be non-null & valid.
         */
        [[nodiscard]]
        virtual
        QPixmap
        renderBackground(const QRect& rect) const;

    private slots:
        void updateNodeConnections(const Node* node) const;
        void wirePointMoved(wire& rawWire, int index);

    private:
        void renderCachedBackground();
        void setupNewItem(Item& item);
        void generateConnections();
        void finishCurrentWire();

        /**
         * Make new wire.
         *
         * @details This makes a new wire. If @p _wireFactory is non-null, the wire factory is used. Otherwise, the
         *          built-in wire type is used.
         *
         * @return The new wire.
         */
        [[nodiscard]]
        std::shared_ptr<Wire>
        make_wire() const;

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

        QPixmap _backgroundPixmap;
        std::function<std::shared_ptr<Wire>()> _wireFactory;
        int _mode = NormalMode;
        std::shared_ptr<Wire> _newWire;
        bool _newWireSegment = false;
        bool _invertWirePosture = true;
        bool _movingNodes = false;
        QPointF _lastMousePos;
        QMap<std::shared_ptr<Item>, QPointF> _initialItemPositions;
        QPointF _initialCursorPosition;
        QUndoStack* _undoStack;
        std::shared_ptr<wire_system::manager> m_wire_manager;
        Item* _highlightedItem = nullptr;
        QTimer* _popupTimer;
        std::shared_ptr<QGraphicsProxyWidget> _popup;
    };

}
