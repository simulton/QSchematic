#pragma once

#include "node.h"

namespace QSchematic::Items
{

    class SubGraph :
        public Node
    {
    public:
        explicit
        SubGraph(QGraphicsItem* parent = nullptr);

        ~SubGraph() override = default;
    };

}
