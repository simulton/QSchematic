#pragma once

#include "item.h"

#include <functional>
#include <memory>

class QString;

namespace QSchematic
{

    class Item;

    class ItemFactory
    {
    public:
        static ItemFactory& instance();

        void setCustomItemsFactory(const std::function<std::shared_ptr<Item>(const gpds::container&)>& factory);
        std::shared_ptr<Item> from_container(const gpds::container& container) const;
        static Item::ItemType extractType(const gpds::container& container);

    private:
        ItemFactory() = default;
        ItemFactory(const ItemFactory& other) = default;
        ItemFactory(ItemFactory&& other) = default;

        std::function<std::shared_ptr<Item>(const gpds::container&)> _customItemFactory;
    };

}
