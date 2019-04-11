#pragma once

#include "wire.h"

namespace QSchematic
{

    class WireRoundedCorners : public QSchematic::Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY(WireRoundedCorners)

    public:
        WireRoundedCorners(int type = Item::WireRoundedCornersType, QGraphicsItem* parent = nullptr);
        virtual ~WireRoundedCorners() override = default;

        virtual bool toXml(QXmlStreamWriter& xml) const override;
        virtual bool fromXml(QXmlStreamReader& reader) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        enum QuarterCircleSegment {
            None,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight
        };
    };

}
