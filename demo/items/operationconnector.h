#pragma once

#include "qschematic/items/connector.h"

class OperationConnector : public QSchematic::Connector
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(OperationConnector)

public:
    OperationConnector(const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
    ~OperationConnector() override = default;

    gpds::container to_container() const override;
    void from_container(const gpds::container& container) override;
    std::shared_ptr<QSchematic::Item> deepCopy() const override;
    std::unique_ptr<QWidget> popup() const override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

protected:
    void copyAttributes(OperationConnector& dest) const;
};
