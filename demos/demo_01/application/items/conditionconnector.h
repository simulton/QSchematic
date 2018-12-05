#pragma once

#include "../../../lib/items/connector.h"

class ConditionConnector : public QSchematic::Connector
{
    Q_OBJECT
    Q_DISABLE_COPY(ConditionConnector)

public:
    ConditionConnector(const QPoint& gridPos = QPoint(), const QString& text = QString(), QGraphicsItem* parent = nullptr);
    virtual ~ConditionConnector() override = default;

    virtual QJsonObject toJson() const override;
    virtual bool fromJson(const QJsonObject& object) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

protected:
    void copyAttributes(ConditionConnector& dest) const;
};
