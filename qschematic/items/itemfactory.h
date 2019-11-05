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

        void setCustomItemsFactory(const std::function<OriginMgrT<Item>(const Gpds::Container&)>& factory);
        OriginMgrT<Item> fromContainer(const Gpds::Container& container) const;
        static Item::ItemType extractType(const Gpds::Container& container);

    private:
        ItemFactory() = default;
        ItemFactory(const ItemFactory& other) = default;
        ItemFactory(ItemFactory&& other) = default;

        std::function<OriginMgrT<Item>(const Gpds::Container&)> _customItemFactory;
    };

}
