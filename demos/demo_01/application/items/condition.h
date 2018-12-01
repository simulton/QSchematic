#pragma once

#include "../../../lib/items/node.h"


class Condition : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(Condition)

public:
    explicit Condition(QGraphicsItem* parent = nullptr);
    virtual ~Condition() override = default;

    virtual QJsonObject toJson() const override;
    virtual bool fromJson(const QJsonObject& object) override;
    virtual QPainterPath shape() const override;
    virtual void update() override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private slots:
    void placeConnectors();
    void calculatePolygon();

private:
    QPolygonF _polygon;
};
