#pragma once

#include <QGraphicsItem>
#include <QGraphicsScene>

namespace QSchematic {

template <typename T>
inline auto dissociate_item(std::shared_ptr<T> item) -> void {
    dissociate_item(item.get());
}

inline auto dissociate_item(QGraphicsItem* item) -> void {
    item->setParentItem(nullptr);

    if ( auto scene = item->scene() ) {
        scene->removeItem(item);
    }
}

template <typename T, template <typename ValT> typename ContainerT>
inline auto dissociate_items(ContainerT<std::shared_ptr<T>> items) -> void {
    for ( auto item : items ) {
        dissociate_item(item.get());
    }
}

}