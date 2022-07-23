#include "itemmimedata.h"

#include <utility>

using namespace QSchematic;

ItemMimeData::ItemMimeData(std::shared_ptr<Item> item) :
    _item(item)
{
}

QStringList ItemMimeData::formats() const
{
    return { MIME_TYPE_NODE };
}

bool ItemMimeData::hasFormat(const QString& mimetype) const
{
    return formats().contains(mimetype);
}

std::shared_ptr<QSchematic::Item> ItemMimeData::item() const
{
    return _item->deepCopy();
}
