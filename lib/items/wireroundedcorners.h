#pragma once

#include "wire.h"

namespace QSchematic
{

    class WireRoundedCorners : public QSchematic::Wire
    {
        Q_OBJECT
        Q_DISABLE_COPY(WireRoundedCorners)

    public:
        WireRoundedCorners(QGraphicsItem* parent = nullptr);
        virtual ~WireRoundedCorners() override = default;

        virtual QJsonObject toJson() const override;
        virtual bool fromJson(const QJsonObject& object) override;
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    private:
        enum ConnectionType {
            Arc,
            SCurve,
            UTurn
        };

        enum ConnectionDirection {
            Up,
            Down,
            Left,
            Right
        };

        enum QuarterCircleSegment {
            None,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight
        };
    };

}
