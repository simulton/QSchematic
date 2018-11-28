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

        virtual QJsonObject toJson() const override;
        virtual bool fromJson(const QJsonObject& object) override;

        virtual QRectF boundingRect() const final;

        void setText(const QString& text);
        QString text() const;
        void setFont(const QFont& font);
        QFont font() const;
        void setConnectionPoint(const QPointF& connectionPoint);    // Parent coordinates
        QRectF textRect() const;

    protected:
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        void calculateTextRect();

        QString _text;
        QFont _font;
        QRectF _textRect;
        QPointF _connectionPoint;   // Parent coordinates
    };

}
