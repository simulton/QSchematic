#pragma once

#include <vector>
#include <memory>
#include <QList>
#include "items/item.h"

namespace QSchematic::ItemUtils
{

template <
    template <typename... T> typename OutContainerT = std::vector,
    typename ContainerT
    >
auto mapItemListToSharedPtrList(ContainerT itemList) -> OutContainerT<std::shared_ptr<Item>>
{
    OutContainerT<std::shared_ptr<Item>> out;

    if constexpr (not std::is_same_v<OutContainerT<Item>, QList<Item>>) {
        out.reserve(itemList.count());
    }

    for (auto& item : itemList) {
        if (auto qsitem = qgraphicsitem_cast<Item*>(item)) {
            out.push_back(qsitem->sharedPtr());
        }
    }

    return out;
}

}
