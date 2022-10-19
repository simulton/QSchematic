#pragma once

#include <QGraphicsItem>
#include <QGraphicsScene>

namespace QSchematic
{

    template <typename T>
    inline
    void
    dissociate_item(std::shared_ptr<T> item)
    {
        dissociate_item(item.get());
    }

    inline
    void
    dissociate_item(QGraphicsItem* item)
    {
        item->setParentItem(nullptr);

        if (auto scene = item->scene())
            scene->removeItem(item);
    }

    template <typename T, template <typename ValT> typename ContainerT>
    inline
    void
    dissociate_items(ContainerT<std::shared_ptr<T>> items)
    {
        for (auto item : items)
            dissociate_item(item.get());
    }

}
