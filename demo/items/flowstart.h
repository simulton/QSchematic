#pragma once

#include "qschematic/items/node.h"

class FlowStart : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FlowStart)

public:
    FlowStart();
    virtual ~FlowStart() override = default;

    virtual gpds::container to_container() const override;
    virtual void from_container(const gpds::container& container) override;
    virtual std::shared_ptr<Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void copyAttributes(FlowStart& dest) const;

private:
    QPolygon _symbolPolygon;
};
