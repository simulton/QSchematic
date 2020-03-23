#pragma once

namespace wire_system
{
    class connectable
    {
    public:
        [[nodiscard]] virtual QPointF position() const = 0;
    };
}