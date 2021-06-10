#pragma once

#include <QString>
#include <QIcon>

namespace QSchematic
{
    class Item;
}

namespace Library
{

    class ItemInfo
    {
    public:
        ItemInfo(QString name_, QIcon icon_, const QSchematic::Item* item_) :
            item(item_),
            name(std::move(name_)),
            icon(std::move(icon_))
        {
        }

        ItemInfo(const ItemInfo& other) = default;
        ItemInfo(ItemInfo&& other) noexcept = default;
        virtual ~ItemInfo() = default;

        ItemInfo& operator=(const ItemInfo& rhs) = default;
        ItemInfo& operator=(ItemInfo&& rhs) noexcept = default;

        const QSchematic::Item* item = nullptr;
        QString name;
        QIcon icon;
    };

}