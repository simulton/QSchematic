#include "subgraph.h"

using namespace QSchematic::Items;

SubGraph::SubGraph(QGraphicsItem* parent) :
    Node(Item::ItemType::SubGraphType, parent)
{
}

void
SubGraph::addChild(std::shared_ptr<Item> item)
{
    if (!item)
        return;

    if (item->parentItem())
        return;

    item->setPos(item->pos() - this->pos());
    item->setParentItem(this);

    m_items.push_back(std::move(item));
}

void
SubGraph::removeChild(std::shared_ptr<Item> item)
{
    if (!item)
        return;

    if (item->parentItem() != this)
        return;

    item->setPos(item->pos() + this->pos());
    item->setParentItem(nullptr);

    // ToDo: C++20 use std::erase
    m_items.erase(
        std::remove(std::begin(m_items), std::end(m_items), item),
        std::end(m_items)
    );
}

std::vector<std::shared_ptr<Item>>
SubGraph::children() const
{
    return m_items;
}

bool
SubGraph::containsChild(const std::shared_ptr<Item>& item) const
{
    if (!item)
        return false;

    const auto it = std::find_if(
        std::cbegin(m_items),
        std::cend(m_items),
        [&item](const auto& i) {
            return i == item;
        }
    );

    return it != std::cend(m_items);
}
