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

        QSize size() const;

        bool addConnector(const QPoint& point);
        QList<QPoint> connectionPoints() const;
        bool isConnectionPoint(const QPoint& gridPoint) const;

        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        QSize _size;

        QList<Connector*> _connectors;
    };

}
