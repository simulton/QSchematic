#pragma once

#include <QList>
#include "item.h"
#include "connector.h"
#include "../types.h"

class QGraphicsSceneMouseEvent;
class QGraphicsSceneHoverEvent;

namespace QSchematic {

    class Connector;

    class Node : public Item
    {
        Q_OBJECT
        Q_DISABLE_COPY(Node)

    public:
        Node(QGraphicsItem* parent = nullptr);
        virtual ~Node() override = default;

        void setSize(const QSize& size);
        void setSize(int width, int height);
        QSize size() const;
        QRect sizeRect() const;
        void setAllowMouseResize(bool enabled);
        bool allowMouseResize() const;
        bool addConnector(Connector* connector);
        QList<QPoint> connectionPoints() const;
        bool isConnectionPoint(const QPoint& gridPoint) const;
        void setConnectorsMovable(bool enabled);
        bool connectorsMovable() const;
        void setConnectorsSnapPolicy(Connector::SnapPolicy policy);
        Connector::SnapPolicy connectorsSnapPolicy() const;
        void setConnectorsSnapToGrid(bool enabled);
        bool connectorsSnapToGrid() const;

        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    protected:
        QMap<RectanglePoint, QRect> resizeHandles() const;

    private:
        enum Mode {
            None,
            Resize
        };

        Mode _mode;
        RectanglePoint _resizeHandle;
        QSize _size;
        bool _allowMouseResize;
        bool _connectorsMovable;
        Connector::SnapPolicy _connectorsSnapPolicy;
        bool _connectorsSnapToGrid;
        QList<Connector*> _connectors;
    };

}
