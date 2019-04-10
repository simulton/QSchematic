#pragma once

#include "../../../lib/items/node.h"

class OperationConnector;

class FlowEnd : public QSchematic::Node
{
    Q_OBJECT
    Q_DISABLE_COPY(FlowEnd)

public:
    FlowEnd();
    virtual ~FlowEnd() override = default;

    virtual bool toXml(QXmlStreamWriter& xml) const override;
    virtual bool fromXml(QXmlStreamReader& reader) override;
    virtual std::unique_ptr<Item> deepCopy() const override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void copyAttributes(FlowEnd& dest) const;

private:
    QPolygon _symbolPolygon;
};
