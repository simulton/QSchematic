#pragma once

#include <memory>
#include <functional>
#include <QGraphicsScene>
#include <QScopedPointer>
#include <QGraphicsProxyWidget>
#include <QUndoStack>
#include "interfaces/json.h"
#include "settings.h"
#include "items/item.h"
#include "items/wire.h"

namespace QSchematic {

    class Node;
    class Connector;
    class WireNet;

    class Scene : public QGraphicsScene, public Json
    {
        Q_OBJECT
        Q_DISABLE_COPY(Scene)

    public:
        enum Mode {
            NormalMode,
            WireMode
        };
        Q_ENUM(Mode)

        explicit Scene(QObject* parent = nullptr);
        virtual ~Scene() override = default;

        virtual QJsonObject toJson() const override;
        virtual bool fromJson(const QJsonObject& object) override;

        void setSettings(const Settings& settings);
        void setWireFactory(const std::function<std::unique_ptr<Wire>()>& factory);
        void setMode(Mode mode);
        void toggleWirePosture();

        void clear();
        bool addItem(Item* item); // Takes ownership
        QList<Item*> items() const;
        QList<Item*> items(Item::ItemType itemType) const;
        QList<Node*> nodes() const;
        bool addWire(Wire* wire);
        bool removeWire(Wire& wire);
        QList<Wire*> wires() const;
        QList<WireNet*> nets() const;
        QList<WireNet*> nets(const WireNet& wireNet) const;
        WireNet* net(const Wire& wire) const;
        QList<WireNet*> netsAt(const QPoint& point);
        QList<QPoint> connectionPoints() const;

        void undo();
        void redo();
        QUndoStack* undoStack() const;

    signals:
        void modeChanged(Mode newMode);

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

    private:
        void renderCachedBackground();
        void setupNewItem(Item* item);
        void addWireNet(std::unique_ptr<WireNet> wireNet);
        QList<Item*> itemsAt(const QPointF& scenePos, Qt::SortOrder order = Qt::DescendingOrder) const;

        QList<WireNet*> _nets;
        Settings _settings;
        QPixmap _backgroundPixmap;
        std::function<std::unique_ptr<Wire>()> _wireFactory;
        Mode _mode;
        QScopedPointer<Wire> _newWire;
        bool _newWireSegment;
        bool _invertWirePosture;
        QPointF _lastMousePos;
        QList<Item*> _selectedItems;
        QScopedPointer<QGraphicsProxyWidget> _popupInfobox;
        QUndoStack* _undoStack;

    private slots:
        void itemMoved(const Item& item, const QVector2D& movedBy);
        void wireNetHighlightChanged(bool highlighted);
        void wirePointMoved(Wire& wire, WirePoint& point);
        void wireMovePoint(const QPoint& point, Wire& wire, const QVector2D& movedBy) const;
        QList<Wire*> wiresConnectedTo(const Node& node, const QVector2D& offset) const;
        void showPopup(const Item& item);
    };

}
