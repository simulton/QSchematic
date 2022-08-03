#pragma once

#include <QList>
#include <QVariant>

namespace Library
{

    template <typename Type, typename Data>
    class ModelItem
    {
    public:
        /**
         * Constructor.
         *
         * @note Takes ownership of @data.
         *
         * @param type
         * @param data
         * @param parent
         */
        ModelItem(const Type& type, Data* data, ModelItem<Type, Data>* parent = nullptr) :
            _type(type),
            _parent(parent),
            _data(data)
        {
        }

        ~ModelItem()
        {
            qDeleteAll(_children);

            delete _data;
        }

        [[nodiscard]]
        Type type() const
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
        const Data* data() const
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
        Type _type;
        Data* _data = nullptr;
        ModelItem* _parent = nullptr;
        QList<ModelItem*> _children;
    };

}
