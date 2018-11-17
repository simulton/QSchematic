#pragma once

#include "label.h"

class QGraphicsSimpleTextItem;

namespace QSchematic {

    class WireNet;

    class WireNetLabel : public Label
    {
    public:
        WireNetLabel(WireNet& net);
        WireNetLabel(const WireNetLabel& other) = delete;
        virtual ~WireNetLabel() override = default;

    protected:
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        WireNet& _net;
    };

}
