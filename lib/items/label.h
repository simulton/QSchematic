#pragma once

#include <QFont>

#include "item.h"

namespace QSchematic {

    class Label : public Item
    {
    public:
        Label(QGraphicsItem* parent = nullptr);
        Label(const Label& other) = delete;
        virtual ~Label() override = default;
        virtual QRectF boundingRect() const final;

        void setText(const QString& text);
        void setFont(const QFont& font);
        void setConnectionPoint(const QPointF& connectionPoint);    // Parent coordinates

    protected:
        QString text() const;
        QFont font() const;
        QRectF textRect() const;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        void calculateTextRect();

        QString _text;
        QFont _font;
        QRectF _textRect;
        QPointF _connectionPoint;   // Parent coordinates
    };

}
