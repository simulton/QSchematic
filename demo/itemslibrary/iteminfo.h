#pragma once

#include <QString>
#include <QIcon>

namespace QSchematic {
    class Item;
}

class ItemInfo
{
public:
    ItemInfo(const QString& name, const QIcon& icon, const QSchematic::Item* item) :
        item(item),
        name(name),
        icon(icon)
    {
    }

    ItemInfo(const ItemInfo& other) = default;
    virtual ~ItemInfo() = default;

    const QSchematic::Item* item;
    QString name;
    QIcon icon;
};
