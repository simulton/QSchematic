#pragma once

#include "../qschematic/items/node.h"

class OperationConnector;

class FlowEnd : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(FlowEnd)

public:
    FlowEnd();
    virtual ~FlowEnd() override = default;

    virtual Gpds::Container toContainer() const override;
    virtual void fromContainer(const Gpds::Container& container) override;
    virtual std::unique_ptr<Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void copyAttributes(FlowEnd& dest) const;

private:
    QPolygon _symbolPolygon;
};
