#pragma once

#include "../connectable.hpp"

#include <QPointF>

struct connector :
    wire_system::connectable
{
    QPointF pos;

    [[nodiscard]]
    QPointF
    position() const override
    {
        return pos;
    }
};