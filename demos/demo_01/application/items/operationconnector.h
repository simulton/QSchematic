#pragma once

#include "../../../lib/items/connector.h"

class OperationConnector : public QSchematic::Connector
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationConnector)

public:
    OperationConnector(const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
    virtual ~OperationConnector() override = default;

    virtual Gds::Container toContainer() const override;
    virtual void fromContainer(const Gds::Container& container) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

protected:
    void copyAttributes(OperationConnector& dest) const;
};
