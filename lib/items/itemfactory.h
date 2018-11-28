#pragma once

#include <functional>
#include <memory>
#include "item.h"

class QJsonObject;
class QString;

namespace QSchematic
{

    class Item;

    class ItemFactory
    {
    public:
        static ItemFactory& instance();

        void setCustomItemsFactory(const std::function<std::unique_ptr<Item>(const QJsonObject&)>& factory);
        std::unique_ptr<Item> fromJson(const QJsonObject& object) const;
        static Item::ItemType extractType(const QJsonObject& object);

    private:
        ItemFactory() = default;
        ItemFactory(const ItemFactory& other) = default;
        ItemFactory(ItemFactory&& other) = default;

        std::function<std::unique_ptr<Item>(const QJsonObject&)> _customItemFactory;
    };

}
