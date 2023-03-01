#pragma once

#include "node.h"

#include <vector>

namespace QSchematic
{
    class Item;
}

namespace QSchematic::Items
{

    class SubGraph :
        public Node
    {
    public:
        explicit
        SubGraph(QGraphicsItem* parent = nullptr);

        ~SubGraph() override = default;

        void
        addChild(std::shared_ptr<Item> item);

        void
        removeChild(std::shared_ptr<Item> item);

        [[nodiscard]]
        std::vector<std::shared_ptr<Item>>
        children() const;

        [[nodiscard]]
        bool
        containsChild(const std::shared_ptr<Item>& item) const;

    private:
        std::vector<std::shared_ptr<Item>> m_items;
    };

}
