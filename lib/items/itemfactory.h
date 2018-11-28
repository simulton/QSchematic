#pragma once

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
        static std::unique_ptr<Item> fromJson(const QJsonObject& object);

    private:
        static Item::ItemType extractType(const QJsonObject& object);

        ItemFactory() = default;
        ItemFactory(const ItemFactory& other) = default;
        ItemFactory(ItemFactory&& other) = default;
    };

}
