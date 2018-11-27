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

        Connector(const QPoint& gridPoint = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
        virtual ~Connector() override = default;

        void setSnapPolicy(SnapPolicy policy);
        SnapPolicy snapPolicy() const;
        void setText(const QString& text);
        QString text() const;
        void setLabelIsVisible(bool enabled);
        bool labelIsVisible() const;
        virtual void update() override;

        QPoint connectionPoint() const;
        virtual QRectF boundingRect() const override;
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        void calculateSymbolRect();
        void calculateTextDirection();

        SnapPolicy _snapPolicy;
        QRectF _symbolRect;
        Direction _textDirection;
        Label* _label;
    };

}
