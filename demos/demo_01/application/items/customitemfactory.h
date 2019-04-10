#pragma once

#include <functional>
#include <memory>
#include <QXmlStreamReader>

class QString;

namespace QSchematic {
    class Item;
}

class CustomItemFactory
{
public:
    static std::unique_ptr<QSchematic::Item> fromXml(const QXmlStreamReader& reader);

private:
    CustomItemFactory() = default;
    CustomItemFactory(const CustomItemFactory& other) = default;
    CustomItemFactory(CustomItemFactory&& other) = default;
};
