#pragma once

#include "qschematic/items/node.h"

#include "itemtypes.h"

namespace QSchematic
{
    class Label;
}

class Operation : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Operation)

public:
    explicit Operation(int type = ::ItemType::OperationType, QGraphicsItem* parent = nullptr);
    ~Operation() override;

    gpds::container to_container() const override;
    void from_container(const gpds::container& container) override;
    std::shared_ptr<QSchematic::Item> deepCopy() const override;
    std::unique_ptr<QWidget> popup() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    std::shared_ptr<QSchematic::Label> label() const;
    void setText(const QString& text);
    QString text() const;

protected:
    void copyAttributes(Operation& dest) const;

private:
    std::shared_ptr<QSchematic::Label> _label;
};
