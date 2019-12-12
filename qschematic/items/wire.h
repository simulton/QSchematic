#pragma once

#include <QAction>
#include "item.h"
#include "wirepoint.h"
#include "wirenet.h"
#include "line.h"

class QVector2D;

namespace QSchematic {

    class Line;

    /**
     * IMPORTANT NOTE: The points coordinates are RELATIVE and in SCENE COORDINATES.
     *                 Wires must be movable so we can move entire groups of stuff.
     */
    class Wire : public Item
    {
        Q_OBJECT

    public:
        Wire(int type = Item::WireType, QGraphicsItem* parent = nullptr);
        virtual ~Wire() override;
        virtual void update() override;

        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;
        virtual std::shared_ptr<Item> deepCopy() const override;
        virtual QRectF boundingRect() const override;
        virtual QPainterPath shape() const override;

        void prependPoint(const QPointF& point);
        void appendPoint(const QPointF& point);
        void insertPoint(int index, const QPointF& point);        // Index of new point
        void removeFirstPoint();
        void removeLastPoint();
        void removePoint(const QPointF& point);
        QVector<WirePoint> wirePointsRelative() const;
        QVector<WirePoint> wirePointsAbsolute() const;
        QVector<QPointF> pointsRelative() const;
        QVector<QPointF> pointsAbsolute() const;
        void simplify();
        void movePointBy(int index, const QVector2D& moveBy);
        void movePointTo(int index, const QPointF& moveTo);
        void moveLineSegmentBy(int index, const QVector2D& moveBy);
        void setPointIsJunction(int index, bool isJunction);
        bool pointIsOnWire(const QPointF& point) const;
        bool connectWire(Wire* wire);
        QList<Wire*> connectedWires();
        void disconnectWire(Wire* wire);
        QVector<int> junctions() const;
        void setNet(const std::shared_ptr<WireNet>& wirenet);
        std::shared_ptr<WireNet> net();
        bool movingWirePoint() const;
        void updatePosition();

        QList<QSchematic::Line> lineSegments() const;

    signals:
        void pointMoved(Wire& wire, WirePoint& point);
        void pointMovedByUser(Wire& wire, int index);
        void pointInserted(int index);
        void pointRemoved(int index);
        void toggleLabelRequested();

    protected:
        void copyAttributes(Wire& dest) const;
        void calculateBoundingRect();
        void setRenameAction(QAction* action);

        virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    private:
        Q_DISABLE_COPY(Wire)

        void removeDuplicatePoints();
        void removeObsoletePoints();

        QVector<WirePoint> _points;
        QList<Wire*> _connectedWires;
        QRectF _rect;
        int _pointToMoveIndex;
        int _lineSegmentToMoveIndex;
        QPointF _prevMousePos;
        QPointF _offset;
        void moveJunctionsToNewSegment(const Line& oldSegment, const Line& newSegment);
        QAction* _renameAction;
        std::shared_ptr<WireNet> _net;
        bool _internalMove;
    };

}
