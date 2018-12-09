#pragma once

#include "item.h"

namespace QSchematic {

    class Label;

    class Connector : public Item
    {
        Q_OBJECT
        Q_DISABLE_COPY(Connector)

    public:
        enum SnapPolicy {
            Anywhere,
            NodeSizerect,
            NodeSizerectOutline,
            NodeShape
        };
        Q_ENUM(SnapPolicy)

        Connector(int type = Item::ConnectorType, const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
        virtual ~Connector() override = default;

        virtual QJsonObject toJson() const override;
        virtual bool fromJson(const QJsonObject& object) override;
        virtual std::unique_ptr<Item> deepCopy() const override;

        void setSnapPolicy(SnapPolicy policy);
        SnapPolicy snapPolicy() const;
        void setText(const QString& text);
        QString text() const;
        void setForceTextDirection(bool enabled);
        bool forceTextDirection() const;
        void setForcedTextDirection(Direction direction);
        Direction textDirection() const;
        void setLabelIsVisible(bool enabled);
        bool labelIsVisible() const;
        virtual void update() override;

        QPoint connectionPoint() const;
        virtual QRectF boundingRect() const override;
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    protected:
        void copyAttributes(Connector& dest) const;
        std::shared_ptr<Label> label() const;

    private:
        void calculateSymbolRect();
        void calculateTextDirection();

        SnapPolicy _snapPolicy;
        QRectF _symbolRect;
        bool _forceTextDirection;
        Direction _textDirection;
        std::shared_ptr<Label> _label;
    };

}
