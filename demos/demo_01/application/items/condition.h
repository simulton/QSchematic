#pragma once

#include "../../../lib/items/node.h"


class Condition : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(Condition)

public:
    explicit Condition(QGraphicsItem* parent = nullptr);
    virtual ~Condition() override = default;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private slots:
    void placeConnectors();
};
