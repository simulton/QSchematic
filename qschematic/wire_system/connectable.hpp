#pragma once

class QPointF;

namespace wire_system
{

    /**
     * An abstract connectable.
     *
     * @details A connectable is anything capable of reporting a 2D position connection point to which wires can connect to.
     */
    struct connectable
    {
        [[nodiscard]]
        virtual
        QPointF
        position() const = 0;
    };

}