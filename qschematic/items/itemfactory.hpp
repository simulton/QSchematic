#pragma once

#include "item.hpp"

#include <functional>
#include <memory>

class QString;

namespace QSchematic::Items
{

    class Item;

    class Factory
    {
    public:
        [[nodiscard]]
        static
        Factory&
        instance();

        void
        setCustomItemsFactory(const std::function<std::shared_ptr<Item>(const gpds::container&)>& factory);

        [[nodiscard]]
        std::shared_ptr<Item>
        from_container(const gpds::container& container) const;

        [[nodiscard]]
        static
        Item::ItemType
        extractType(const gpds::container& container);

    private:
        Factory() = default;
        Factory(const Factory& other) = default;
        Factory(Factory&& other) = default;

        std::function<std::shared_ptr<Item>(const gpds::container&)> _customItemFactory;
    };

}
