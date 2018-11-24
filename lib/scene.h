#pragma once

#include <QGraphicsScene>
#include <QScopedPointer>
#include <QGraphicsProxyWidget>
#include "settings.h"
#include "items/item.h"
#include "items/wire.h"

namespace QSchematic {

    class Node;
    class Connector;
    class WireNet;

    class Scene : public QGraphicsScene
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

        void setSettings(const Settings& settings);
        void setMode(Mode mode);
        void toggleWirePosture();

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

    signals:
        void modeChanged(Mode newMode);

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        virtual void keyPressEvent(QKeyEvent* event) override;
        virtual void keyReleaseEvent(QKeyEvent* event) override;
        virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

    private:
        QList<QPoint> connectionPoints() const;
        void addWireNet(WireNet* wireNet);
        void updateWireJunctions();

        QList<WireNet*> _nets;
        Settings _settings;
        Mode _mode;
        QScopedPointer<Wire> _newWire;
        bool _newWireSegment;
        bool _invertWirePosture;
        QPointF _lastMousePos;
        QScopedPointer<QGraphicsProxyWidget> _popupInfobox;

    private slots:
        void wireNetHighlightChanged(bool highlighted);
        void pointMoved(Wire& wire, WirePoint& point);
        void showPopup(const Item& item);
    };

}
