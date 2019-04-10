#pragma once

#include "../../../lib/items/wireroundedcorners.h"

class FancyWire : public QSchematic::WireRoundedCorners
{
    Q_OBJECT
    Q_DISABLE_COPY(FancyWire)

public:
    FancyWire(QGraphicsItem* parent = nullptr);
    virtual ~FancyWire() override = default;

    virtual bool toXml(QXmlStreamWriter& xml) const override;
    virtual bool fromXml(QXmlStreamReader& reader) override;
    virtual std::unique_ptr<QSchematic::Item> deepCopy() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void copyAttributes(FancyWire& dest) const;
};
