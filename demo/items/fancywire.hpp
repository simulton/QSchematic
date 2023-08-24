#pragma once

#include <qschematic/items/wireroundedcorners.hpp>

class FancyWire : public QSchematic::Items::WireRoundedCorners
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FancyWire)

public:
    explicit FancyWire(QGraphicsItem* parent = nullptr);
    ~FancyWire() override = default;

    gpds::container to_container() const override;
    void from_container(const gpds::container& container) override;
    std::shared_ptr<QSchematic::Items::Item> deepCopy() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void copyAttributes(FancyWire& dest) const;
};
