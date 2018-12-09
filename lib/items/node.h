#pragma once

#include <QList>
#include "item.h"
#include "connector.h"
#include "../types.h"

class QGraphicsSceneMouseEvent;
class QGraphicsSceneHoverEvent;

namespace QSchematic {

    class Connector;
    class Label;

    class Node : public Item
    {
        Q_OBJECT
        Q_DISABLE_COPY(Node)

    signals:
        void sizeChanged();

    public:
        enum ResizePolicy {
            VerticalOddOnly    = 0x01,
            VerticalEvenOnly   = 0x02,
            HorizontalOddOnly  = 0x04,
            HorizontalEvenOnly = 0x08
        };
        Q_ENUM(ResizePolicy)

        enum Mode {
            None,
            Resize
        };
        Q_ENUM(Mode)

        Node(int type = Item::NodeType, QGraphicsItem* parent = nullptr);
        virtual ~Node() override = default;

        virtual QJsonObject toJson() const override;
        virtual bool fromJson(const QJsonObject& object) override;
        virtual std::unique_ptr<Item> deepCopy() const override;

        Mode mode() const;
        void setSize(const QSize& size);
        void setSize(int width, int height);
        QSize size() const;
        QRect sizeRect() const;
        QRect sizeSceneRect() const;
        void setMouseResizePolicy(ResizePolicy policy);
        ResizePolicy mouseResizePolicy() const;
        void setAllowMouseResize(bool enabled);
        bool allowMouseResize() const;
        bool addConnector(const std::shared_ptr<Connector>& connector);
        bool removeConnector(const std::shared_ptr<Connector>& connector);
        void clearConnectors();
        QList<std::shared_ptr<Connector>> connectors() const;
        QList<QPoint> connectionPoints() const;
        bool isConnectionPoint(const QPoint& gridPos) const;
        void setConnectorsMovable(bool enabled);
        bool connectorsMovable() const;
        void setConnectorsSnapPolicy(Connector::SnapPolicy policy);
        Connector::SnapPolicy connectorsSnapPolicy() const;
        void setConnectorsSnapToGrid(bool enabled);
        bool connectorsSnapToGrid() const;
        std::shared_ptr<QSchematic::Label> label() const;

        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    protected:
        void copyAttributes(Node& dest) const;
        QMap<RectanglePoint, QRect> resizeHandles() const;
        void paintResizeHandles(QPainter& painter);

    private:
        std::shared_ptr<Label> _label;
        Mode _mode;
        QPoint _lastMousePosWithGridMove;
        RectanglePoint _resizeHandle;
        QSize _size;
        ResizePolicy _mouseResizePolicy;
        bool _allowMouseResize;
        bool _connectorsMovable;
        Connector::SnapPolicy _connectorsSnapPolicy;
        bool _connectorsSnapToGrid;
        QList<std::shared_ptr<Connector>> _connectors;
    };

}
