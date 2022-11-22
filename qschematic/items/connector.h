#pragma once

#include "item.h"
#include "../wire_system/connectable.h"

namespace QSchematic {

    class Label;
    class Wire;

    class Connector :
        public Item,
        public wire_system::connectable
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Connector)

    public:
        enum SnapPolicy {
            Anywhere,
            NodeSizerect,
            NodeSizerectOutline
        };
        Q_ENUM(SnapPolicy)

        Connector(int type = Item::ConnectorType, const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
        ~Connector() override;

        gpds::container to_container() const override;
        void from_container(const gpds::container& container) override;
        std::shared_ptr<Item> deepCopy() const override;

        void setSnapPolicy(SnapPolicy policy);
        SnapPolicy snapPolicy() const;
        void setText(const QString& text);
        QString text() const;
        void setForceTextDirection(bool enabled);
        bool forceTextDirection() const;
        void setForcedTextDirection(Direction direction);
        Direction textDirection() const;
        void update() override;

        QPointF connectionPoint() const;
        std::shared_ptr<Label> label() const;
        void alignLabel();
        QRectF boundingRect() const override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        // Connectable
        QPointF position() const override;

    protected:
        void copyAttributes(Connector& dest) const;
        QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    private:
        void calculateSymbolRect();
        void calculateTextDirection();
        void disconnect_all_wires();
        void notify_wire_manager();

        SnapPolicy _snapPolicy;
        QRectF _symbolRect;
        bool _forceTextDirection;
        Direction _textDirection;
        std::shared_ptr<Label> _label;
    };

}
