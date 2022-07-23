#pragma once

#include <QPointF>

class QPointF;

namespace wire_system
{
    class connectable
    {
    public:
        [[nodiscard]] virtual QPointF position() const = 0;
    };
}