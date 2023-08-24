#pragma once

#include "operation.hpp"

class OperationConnector;

class FlowEnd :
    public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FlowEnd)

public:
    FlowEnd();
    ~FlowEnd() override = default;

    gpds::container to_container() const override;
    void from_container(const gpds::container& container) override;
    std::shared_ptr<Item> deepCopy() const override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void copyAttributes(FlowEnd& dest) const;

private:
    QPolygon _symbolPolygon;
};
