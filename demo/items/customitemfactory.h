#pragma once

#include <functional>
#include <memory>

class QString;

namespace QSchematic {
    class Item;
}

namespace Gpds {
    class Container;
}

class CustomItemFactory
{
public:
    static std::shared_ptr<QSchematic::Item> fromContainer(const Gpds::Container& container);

private:
    CustomItemFactory() = default;
    CustomItemFactory(const CustomItemFactory& other) = default;
    CustomItemFactory(CustomItemFactory&& other) = default;
};
