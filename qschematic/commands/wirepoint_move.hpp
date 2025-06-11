#pragma once

#include "base.hpp"

#include <QVector>
#include <QVector2D>

#include <memory>

class QVector2D;

namespace QSchematic::Items
{
    class Wire;
}

namespace QSchematic::Commands
{

    class WirepointMove :
        public Base
    {
    public:
        WirepointMove(
            std::shared_ptr<Items::Wire> wire,
            int index,
            const QPointF& pos,     // New wire point position (absolute/scene position)
            QUndoCommand* parent = nullptr
        );

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        struct WirePoint {
            int pointIndex = -1;
            QPointF pos;
        };

        std::shared_ptr<Items::Wire> _wire;
        WirePoint _old;
        WirePoint _new;
    };

}
