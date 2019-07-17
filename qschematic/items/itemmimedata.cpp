#include <utility>
#include "itemmimedata.h"

using namespace QSchematic;

ItemMimeData::ItemMimeData(std::unique_ptr<Item> item) :
    _item(std::move(item))
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

std::unique_ptr<QSchematic::Item> ItemMimeData::item() const
{
    return _item->deepCopy();
}
