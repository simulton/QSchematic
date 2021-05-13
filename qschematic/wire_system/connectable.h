#pragma once

#include "qschematic_export.h"

namespace wire_system
{
    class QSCHEMATIC_EXPORT connectable
    {
    public:
        [[nodiscard]] virtual QPointF position() const = 0;
    };
}