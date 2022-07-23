#pragma once

#include <qschematic/items/item.h>

#include <functional>
#include <memory>

class QString;

namespace QSchematic {
    class Item;
}

namespace gpds {
    class container;
}

class CustomItemFactory
{
public:
    static std::shared_ptr<QSchematic::Item> from_container(const gpds::container& container);

private:
    CustomItemFactory() = default;
    CustomItemFactory(const CustomItemFactory& other) = default;
    CustomItemFactory(CustomItemFactory&& other) = default;
};
