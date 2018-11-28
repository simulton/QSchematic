#pragma once

#include <functional>
#include <memory>

class QJsonObject;
class QString;

namespace QSchematic {
    class Item;
}

class CustomItemFactory
{
public:
    static std::unique_ptr<QSchematic::Item> fromJson(const QJsonObject& object);

private:
    CustomItemFactory() = default;
    CustomItemFactory(const CustomItemFactory& other) = default;
    CustomItemFactory(CustomItemFactory&& other) = default;
};
