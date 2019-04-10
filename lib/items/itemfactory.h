#pragma once

#include <functional>
#include <memory>
#include "item.h"

class QString;

namespace QSchematic
{

    class Item;

    class ItemFactory
    {
    public:
        static ItemFactory& instance();

        void setCustomItemsFactory(const std::function<std::unique_ptr<Item>(const QXmlStreamReader&)>& factory);
        std::unique_ptr<Item> fromXml(const QXmlStreamReader& reader) const;
        static Item::ItemType extractType(const QXmlStreamReader& reader);

    private:
        ItemFactory() = default;
        ItemFactory(const ItemFactory& other) = default;
        ItemFactory(ItemFactory&& other) = default;

        std::function<std::unique_ptr<Item>(const QXmlStreamReader&)> _customItemFactory;
    };

}
