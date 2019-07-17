#pragma once

#include <QList>
#include <QVariant>

template <typename T>
class ItemsLibraryModelItem
{
public:
    explicit ItemsLibraryModelItem(const T& type, void* data, ItemsLibraryModelItem<T>* parent = nullptr) : _type(type)
    {
        _data = data;
        _parent = parent;
    }

    ~ItemsLibraryModelItem()
    {
        qDeleteAll(_children);
    }

    T type() const
    {
        return _type;
    }

    void appendChild(ItemsLibraryModelItem* child)
    {
        _children.append(child);
    }

    void deleteChild(ItemsLibraryModelItem* child)
    {
        _children.removeAll(child);
        delete child;
    }

    void deleteChild(int index)
    {
        deleteChild(_children.at(index));
    }

    ItemsLibraryModelItem* child(int row)
    {
        return _children.value(row);
    }

    QList<ItemsLibraryModelItem*> children() const
    {
        return _children;
    }

    int childCount() const
    {
        return _children.count();
    }

    void* data() const
    {
        return _data;
    }

    int row() const
    {
        if (_parent) {
            return _parent->children().indexOf(const_cast<ItemsLibraryModelItem*>(this));
        }

        return 0;
    }

    ItemsLibraryModelItem* parent()
    {
        return _parent;
    }

private:
    T _type;
    ItemsLibraryModelItem* _parent;
    QList<ItemsLibraryModelItem*> _children;
    void* _data;
};
