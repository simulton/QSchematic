#pragma once

#include <QIcon>
#include <QString>

#include <qschematic/items/item.hpp>

namespace QSchematic::Items
{
    class Item;
}

namespace Library
{

    /**
     * @note Takes ownership of @p item.
     */
    class ItemInfo
    {
    public:
        /**
         * @note Takes ownership of @p item_.
         *
         * @param name_
         * @param icon_
         * @param item_
         */
        ItemInfo(QString name_, QIcon icon_, const QSchematic::Items::Item* item_) :
            item(item_),
            name(std::move(name_)),
            icon(std::move(icon_))
        {
        }

        ItemInfo(const ItemInfo& other) = default;
        ItemInfo(ItemInfo&& other) noexcept = default;

        virtual
        ~ItemInfo() noexcept
        {
            delete item;
        }

        ItemInfo& operator=(const ItemInfo& rhs) = default;
        ItemInfo& operator=(ItemInfo&& rhs) noexcept = default;

        const QSchematic::Items::Item* item = nullptr;
        QString name;
        QIcon icon;
    };

}