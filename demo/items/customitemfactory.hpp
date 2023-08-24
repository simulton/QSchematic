#pragma once

#include <qschematic/items/item.hpp>

#include <functional>
#include <memory>

class QString;

namespace QSchematic::Items
{
    class Item;
}

namespace gpds
{
    class container;
}

class CustomItemFactory
{
public:
    static std::shared_ptr<QSchematic::Items::Item> from_container(const gpds::container& container);

private:
    CustomItemFactory() = default;
    CustomItemFactory(const CustomItemFactory& other) = default;
    CustomItemFactory(CustomItemFactory&& other) = default;
};
