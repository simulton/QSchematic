#pragma once

#include "../connectable.hpp"

#include <QPointF>

struct connector :
    wire_system::connectable
{
    QPointF pos;

    QPointF
    position() const override
    {
        return pos;
    }
};