#pragma once

#include "wire.h"
#include "qschematic_export.h"

namespace QSchematic
{

    class QSCHEMATIC_EXPORT WireRoundedCorners :
        public QSchematic::Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY(WireRoundedCorners)

    public:
        WireRoundedCorners(int type = Item::WireRoundedCornersType, QGraphicsItem* parent = nullptr);
        virtual ~WireRoundedCorners() override = default;

        virtual gpds::container to_container() const override;
        virtual void from_container(const gpds::container& container) override;
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
