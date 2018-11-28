#pragma once

#include "../../../lib/items/node.h"


class Operation : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(Operation)

public:
    explicit Operation(QGraphicsItem* parent = nullptr);
    virtual ~Operation() override = default;

    virtual QJsonObject toJson() const override;
    virtual bool fromJson(const QJsonObject& object) override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
};
