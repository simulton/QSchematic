#pragma once

#include "../../../lib/items/node.h"
#include "itemtypes.h"

namespace QSchematic
{
    class Label;
}

class Operation : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(Operation)

public:
    explicit Operation(int type = ::ItemType::OperationType, QGraphicsItem* parent = nullptr);
    virtual ~Operation() override = default;

    virtual Gpds::Container toContainer() const override;
    virtual void fromContainer(const Gpds::Container& container) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    std::shared_ptr<QSchematic::Label> label() const;
    void setText(const QString& text);
    QString text() const;

protected:
    void copyAttributes(Operation& dest) const;

private:
    std::shared_ptr<QSchematic::Label> _label;
};
