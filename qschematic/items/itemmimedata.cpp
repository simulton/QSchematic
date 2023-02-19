#include "itemmimedata.h"

#include <utility>

using namespace QSchematic::Items;

MimeData::MimeData(std::shared_ptr<Item> item) :
    _item(item)
{
}

QStringList MimeData::formats() const
{
    return { MIME_TYPE_NODE };
}

bool MimeData::hasFormat(const QString& mimetype) const
{
    return formats().contains(mimetype);
}

std::shared_ptr<Item> MimeData::item() const
{
    if (_item)
        return _item->deepCopy();

    return { };
}
