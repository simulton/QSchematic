#pragma once

#include <qschematic/items/item.h>

#include <QIcon>
#include <QString>

namespace QSchematic
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
        ItemInfo(QString name_, QIcon icon_, const QSchematic::Item* item_) :
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

        const QSchematic::Item* item = nullptr;
        QString name;
        QIcon icon;
    };

}