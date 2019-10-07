#pragma once

#include <functional>
#include <memory>
#include "item.h"

class QString;

namespace QSchematic
{

    class Item;

    class ItemFactory
    {
    public:
        static ItemFactory& instance();

        void setCustomItemsFactory(const std::function<std::shared_ptr<Item>(const Gpds::Container&)>& factory);
        std::shared_ptr<Item> fromContainer(const Gpds::Container& container) const;
        static Item::ItemType extractType(const Gpds::Container& container);

    private:
        ItemFactory() = default;
        ItemFactory(const ItemFactory& other) = default;
        ItemFactory(ItemFactory&& other) = default;

        std::function<std::shared_ptr<Item>(const Gpds::Container&)> _customItemFactory;
    };

}
