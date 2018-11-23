#pragma once

#include <QList>
#include "item.h"

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
        QRect bodyRect() const; // relative (x & y == 0)

        bool addConnector(const QPoint& point, const QString& text = QString());
        QList<QPoint> connectionPoints() const;
        bool isConnectionPoint(const QPoint& gridPoint) const;
        void setConnectorsMovable(bool enabled);
        bool connectorsMovable() const;

        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        QSize _size;
        bool _connectorsMovable;

        QList<Connector*> _connectors;
    };

}
