#pragma once

#include "../../../lib/items/connector.h"

class MyConnector : public QSchematic::Connector
{
    Q_OBJECT
    Q_DISABLE_COPY(MyConnector)

public:
    MyConnector(const QPoint& gridPoint = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
    virtual ~MyConnector() override = default;

    virtual QJsonObject toJson() const override;
    virtual bool fromJson(const QJsonObject& object) override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
};
