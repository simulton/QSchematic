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
    static QSchematic::OriginMgrT<QSchematic::Item> fromContainer(const Gpds::Container& container);

private:
    CustomItemFactory() = default;
    CustomItemFactory(const CustomItemFactory& other) = default;
    CustomItemFactory(CustomItemFactory&& other) = default;
};
