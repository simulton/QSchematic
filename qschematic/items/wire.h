#pragma once

#include <QAction>

#include "wire_system/point.h"
#include "wire_system/wire.h"
#include "item.h"
#include "wirenet.h"
#include "qschematic_export.h"

class QVector2D;

namespace QSchematic {

    /**
     * IMPORTANT NOTE: The points coordinates are RELATIVE and in SCENE COORDINATES.
     *                 Wires must be movable so we can move entire groups of stuff.
     */
    class QSCHEMATIC_EXPORT Wire :
        public Item,
        public wire_system::wire
    {
        Q_OBJECT

    public:
        Wire(int type = Item::WireType, QGraphicsItem* parent = nullptr);
        ~Wire() override;
        void update() override;

        gpds::container to_container() const override;
        void from_container(const gpds::container& container) override;
        std::shared_ptr<Item> deepCopy() const override;
        QRectF boundingRect() const override;
        QPainterPath shape() const override;

        void prepend_point(const QPointF& point) override;
        void append_point(const QPointF& point) override;
        void insert_point(int index, const QPointF& point) override;        // Index of new point
        void removeFirstPoint();
        void removeLastPoint();
        QVector<point> wirePointsRelative() const;
        QVector<QPointF> pointsRelative() const;
        QVector<QPointF> pointsAbsolute() const;
        void move_point_to(int index, const QPointF& moveTo) override;
        bool movingWirePoint() const;
        void rename_net();

    signals:
        void pointMoved(Wire& wire, point& point);
        void toggleLabelRequested();

    protected:
        void copyAttributes(Wire& dest) const;
        void calculateBoundingRect();
        void setRenameAction(QAction* action);

        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
        void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
        QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;
        void about_to_change() override;
        void has_changed() override;
        void add_segment(int index) override;

    private:
        Q_DISABLE_COPY_MOVE(Wire)

        void label_to_cursor(const QPointF& scenePos, std::shared_ptr<Label>& label) const;

        QRectF _rect;
        int _pointToMoveIndex;
        int _lineSegmentToMoveIndex;
        QPointF _prevMousePos;
        QPointF _offset;
        QAction* _renameAction;
    };

}
