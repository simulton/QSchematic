#pragma once

#include "item.h"

namespace QSchematic {

    class Connector : public Item
    {
        Q_OBJECT
        Q_DISABLE_COPY(Connector)

    public:
        Connector(QGraphicsItem* parent = nullptr);
        virtual ~Connector() override = default;

        void setText(const QString& text);
        QString text() const;

        QPoint connectionPoint() const;
        virtual QRectF boundingRect() const override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        QPointF snapPointToPath(const QPointF& point, const QPainterPath& path) const;
        void calculateSymbolRect();
        void calculateTextRect();

        QString _text;
        QRectF _symbolRect;
        QRectF _textRect;
        Direction _textDiretion;
    };

}
