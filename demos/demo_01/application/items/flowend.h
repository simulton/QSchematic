#pragma once

#include "../../../lib/items/node.h"

class OperationConnector;

class FlowEnd : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(FlowEnd)

public:
    FlowEnd();
    virtual ~FlowEnd() override = default;

    virtual QJsonObject toJson() const override;
    virtual bool fromJson(const QJsonObject& object) override;
    virtual std::unique_ptr<Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void copyAttributes(FlowEnd& dest) const;

private:
    std::shared_ptr<OperationConnector> _connector;
    QPolygon _symbolPolygon;
};
