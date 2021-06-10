#pragma once

#include <QList>
#include <QVariant>

namespace Library
{

    template <typename T>
    class ModelItem
    {
    public:
        ModelItem(const T& type, void* data, ModelItem<T>* parent = nullptr) :
            _type(type),
            _parent(parent),
            _data(data)
        {
        }

        ~ModelItem()
        {
            qDeleteAll(_children);
        }

        [[nodiscard]]
        T type() const
        {
            return _type;
        }

        void appendChild(ModelItem* child)
        {
            _children.append(child);
        }

        void deleteChild(ModelItem* child)
        {
            _children.removeAll(child);
            delete child;
        }

        void deleteChild(int index)
        {
            deleteChild(_children.at(index));
        }

        [[nodiscard]]
        ModelItem* child(int row)
        {
            return _children.value(row);
        }

        [[nodiscard]]
        QList<ModelItem*> children() const
        {
            return _children;
        }

        [[nodiscard]]
        int childCount() const
        {
            return _children.count();
        }

        [[nodiscard]]
        void* data() const
        {
            return _data;
        }

        [[nodiscard]]
        int row() const
        {
            if (_parent) {
                return _parent->children().indexOf(const_cast<ModelItem*>(this));
            }

            return 0;
        }

        [[nodiscard]]
        ModelItem* parent()
        {
            return _parent;
        }

    private:
        T _type;
        ModelItem* _parent = nullptr;
        QList<ModelItem*> _children;
        void* _data = nullptr;
    };

}
