#pragma once

#include "../../../lib/items/connector.h"

class OperationConnector : public QSchematic::Connector
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationConnector)

public:
    OperationConnector(const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
    virtual ~OperationConnector() override = default;

    virtual bool toXml(QXmlStreamWriter& xml) const override;
    virtual bool fromXml(QXmlStreamReader& reader) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

protected:
    void copyAttributes(OperationConnector& dest) const;
};
