#pragma once

#include "../items/node.hpp"

#include <memory>

namespace test
{
    [[nodiscard]]
    inline
    std::shared_ptr<QSchematic::Items::Node>
    make_node(int x, int y)
    {
        auto node = std::make_shared<QSchematic::Items::Node>();
        node->setPos(x, y);

        return node;
    }
}
