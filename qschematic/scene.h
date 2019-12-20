#pragma once

#include <memory>
#include <functional>
#include <QGraphicsScene>
#include <QUndoStack>
#include <gpds/serialize.hpp>
#include "settings.h"
#include "items/item.h"
#include "items/wire.h"
//#include "utils/itemscustodian.h"

namespace QSchematic {

    class Node;
    class Connector;
    class WireNet;

    class Scene : public QGraphicsScene, public gpds::serialize
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

        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;

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
        QList<std::shared_ptr<Item>> itemsAt(const QPointF& scenePos, Qt::SortOrder order = Qt::DescendingOrder) const;
        std::vector<std::shared_ptr<Item>> selectedItems() const;
        std::vector<std::shared_ptr<Item>> selectedTopLevelItems() const;
        QList<std::shared_ptr<Node>> nodes() const;
        std::shared_ptr<Node> nodeFromConnector(const Connector& connector) const;
        bool addWire(const std::shared_ptr<Wire>& wire);
        bool removeWire(const std::shared_ptr<Wire> wire);
        QList<std::shared_ptr<Wire>> wires() const;
        QList<std::shared_ptr<WireNet>> nets() const;
        QList<std::shared_ptr<WireNet>> nets(const std::shared_ptr<WireNet> wireNet) const;
        std::shared_ptr<WireNet> net(const std::shared_ptr<Wire> wire) const;
        QList<std::shared_ptr<WireNet>> netsAt(const QPoint& point);
        QList<QPointF> connectionPoints() const;
        QList<std::shared_ptr<Connector>> connectors() const;
        std::shared_ptr<WireNet> netFromWire(const std::shared_ptr<Wire>& wire) const;
        void removeWireNet(std::shared_ptr<WireNet> net);

        void undo();
        void redo();
        QUndoStack* undoStack() const;
        void addWireNet(const std::shared_ptr<WireNet> wireNet);

    signals:
        void modeChanged(int newMode);
        void isDirtyChanged(bool isDirty);
        void itemAdded(const std::shared_ptr<const Item> item);
        void itemRemoved(const std::shared_ptr<const Item> item);

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
        bool mergeNets(std::shared_ptr<WireNet>& net, std::shared_ptr<WireNet>& otherNet);
        void moveWireToNet(std::shared_ptr<Wire>& rawWire, std::shared_ptr<WireNet>& newNet) const;
        void connectWire(const std::shared_ptr<Wire>& wire, std::shared_ptr<Wire>& rawWire);
        void disconnectWire(const std::shared_ptr<Wire>& wire, const std::shared_ptr<Wire>& otherWire);
        QVector<std::shared_ptr<Wire>> wiresConnectedTo(const std::shared_ptr<Wire>& wire) const;

        // TODO add to "central" sh-ptr management
        QList<std::shared_ptr<Item>> _keep_alive_an_event_loop;
        /**
         * Used to store a list of "Top-Level" items. These are the only items
         * moved by the scene. Scene::addItem automatically adds the items to
         * this list. Items that are children of another Item should
         * not be in the list.
         */
        QList<std::shared_ptr<Item>> _items;
        QList<std::shared_ptr<WireNet>> _nets;

        // Note: haven't investigated destructor specification, but it seems
        // this can be skipped, although it would be: explicit, more efficient,
        // and possibly required in more complex destruction scenarios — but
        // we're skipping that extra work now / ozra
        //
        // ItemUtils::ItemsCustodian<Item> _items;
        // ItemUtils::ItemsCustodian<WireNet> _nets;

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

    private slots:
        void wireNetHighlightChanged(bool highlighted);
        void wirePointMoved(Wire& wire, WirePoint& point);
        void wirePointMovedByUser(Wire& rawWire, int point);
        void updateNodeConnections(const Node* node) const;
    };

}
