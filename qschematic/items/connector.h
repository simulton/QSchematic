#pragma once

#include "item.h"
#include "wire_system/connectable.h"
#include "qschematic_export.h"

namespace QSchematic {

    class Label;
    class Wire;

    class QSCHEMATIC_EXPORT Connector :
        public Item,
        public wire_system::connectable
    {
        Q_OBJECT
        Q_DISABLE_COPY(Connector)

    public:
        enum SnapPolicy {
            Anywhere,
            NodeSizerect,
            NodeSizerectOutline
        };
        Q_ENUM(SnapPolicy)

        Connector(int type = Item::ConnectorType, const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
        virtual ~Connector() override;

#ifdef USE_GPDS
        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;
#endif
        virtual std::shared_ptr<Item> deepCopy() const override;

        void setSnapPolicy(SnapPolicy policy);
        SnapPolicy snapPolicy() const;
        void setText(const QString& text);
        QString text() const;
        void setForceTextDirection(bool enabled);
        bool forceTextDirection() const;
        void setForcedTextDirection(Direction direction);
        Direction textDirection() const;
        virtual void update() override;

        QPointF connectionPoint() const;
        std::shared_ptr<Label> label() const;
        void alignLabel();
        virtual QRectF boundingRect() const override;
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        // Connectable
        QPointF position() const override;

    protected:
        void copyAttributes(Connector& dest) const;

    private:
        void calculateSymbolRect();
        void calculateTextDirection();
        void notify_wire_manager();

        SnapPolicy _snapPolicy;
        QRectF _symbolRect;
        bool _forceTextDirection;
        Direction _textDirection;
        std::shared_ptr<Label> _label;
    };

}
