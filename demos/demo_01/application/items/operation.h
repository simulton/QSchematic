#pragma once

#include "../../../lib/items/node.h"

namespace QSchematic
{
    class Label;
}

class Operation : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(Operation)

public:
    explicit Operation(QGraphicsItem* parent = nullptr);
    virtual ~Operation() override = default;

    virtual QJsonObject toJson() const override;
    virtual bool fromJson(const QJsonObject& object) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    std::shared_ptr<QSchematic::Label> label() const;

protected:
    void copyAttributes(Operation& dest) const;

private:
    std::shared_ptr<QSchematic::Label> _label;

private slots:
    void repositionLabel();
};
