#pragma once

#include "items/item.h"

#include <QObject>

#include <unordered_map>
#include <memory>

namespace QSchematic {

namespace ItemUtils
{

template <typename CustodyItemT>
class ItemsCustodian
{
public:
    /**
     * Make sure parent is removed for all items in custody, since they will be
     * deleted by managed pointer, to ensure no double delete occurs by Qt
     */
    ~ItemsCustodian()
    {
        if constexpr (std::is_base_of_v<QSchematic::Item, CustodyItemT>) {
            for ( auto const& item : _custody_items ) {
                item->setParentItem(nullptr);
            }
        }
        else if constexpr (std::is_base_of_v<QObject, CustodyItemT>) {
            for ( auto const& item : _custody_items ) {
                item->setParent(nullptr);
            }
        }
        else {
            return;
        }
    }

    auto takeCustody(std::shared_ptr<CustodyItemT> item) -> void
    {
        _custody_items.insert_or_assign(item.get(), item);
    }

    auto releaseCustody(std::shared_ptr<CustodyItemT> item) -> void
    {
        _custody_items.erase(item.get());
    }

    auto clear() -> void {
        _custody_items.clear();
    }

private:
    std::unordered_map<CustodyItemT, std::shared_ptr<CustodyItemT>> _custody_items;

};

}

}
