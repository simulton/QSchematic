#pragma once

#include <functional>
#include <memory>
#include "../qschematic/items/item.h"

class QString;

namespace Gpds {
    class Container;
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
