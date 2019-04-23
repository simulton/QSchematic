#pragma once

#include "../../../lib/items/node.h"
#include "itemtypes.h"

class Operation : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(Operation)

public:
    explicit Operation(int type = ::ItemType::OperationType, QGraphicsItem* parent = nullptr);
    virtual ~Operation() override = default;

    virtual Gds::Container toContainer() const override;
    virtual void fromContainer(const Gds::Container& container) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

protected:
    void copyAttributes(Operation& dest) const;
};
