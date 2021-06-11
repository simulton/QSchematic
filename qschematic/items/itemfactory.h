#pragma once

#include <functional>
#include <memory>
#include "item.h"
#include "qschematic_export.h"

class QString;

namespace QSchematic
{

    class Item;

    class QSCHEMATIC_EXPORT ItemFactory
    {
    public:
        static ItemFactory& instance();
#ifdef USE_GPDS
        void setCustomItemsFactory(const std::function<std::shared_ptr<Item>(const gpds::container&)>& factory);
        std::shared_ptr<Item> from_container(const gpds::container& container) const;
        static Item::ItemType extractType(const gpds::container& container);
#endif

    private:
        ItemFactory() = default;
        ItemFactory(const ItemFactory& other) = default;
        ItemFactory(ItemFactory&& other) = default;
#ifdef USE_GPDS
        std::function<std::shared_ptr<Item>(const gpds::container&)> _customItemFactory;
#endif
    };

}
