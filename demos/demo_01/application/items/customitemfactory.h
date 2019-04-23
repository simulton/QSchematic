#pragma once

#include <functional>
#include <memory>

class QString;

namespace QSchematic {
    class Item;
}

namespace Gds {
    class Container;
}

class CustomItemFactory
{
public:
    static std::unique_ptr<QSchematic::Item> fromContainer(const Gds::Container& container);

private:
    CustomItemFactory() = default;
    CustomItemFactory(const CustomItemFactory& other) = default;
    CustomItemFactory(CustomItemFactory&& other) = default;
};
