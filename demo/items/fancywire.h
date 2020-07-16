#pragma once

#include "qschematic/items/wireroundedcorners.h"

class FancyWire : public QSchematic::WireRoundedCorners
{
    Q_OBJECT
    Q_DISABLE_COPY(FancyWire)

public:
    FancyWire(QGraphicsItem* parent = nullptr);
    virtual ~FancyWire() override = default;

    virtual gpds::container to_container() const override;
    virtual void from_container(const gpds::container& container) override;
    virtual std::shared_ptr<QSchematic::Item> deepCopy() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void copyAttributes(FancyWire& dest) const;
};
