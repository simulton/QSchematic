#pragma once

#include <QVector>
#include <QVariant>

class TreeItem
{
public:
    explicit TreeItem(const QVector<QVariant>& data, TreeItem* parentItem = nullptr);
    virtual ~TreeItem();

    void clear();
    void setData(QVector<QVariant>&& data);
    void appendChild(TreeItem* child);

    [[nodiscard]] TreeItem* child(int row);
    [[nodiscard]] int childCount() const;
    [[nodiscard]] int columnCount() const;
    [[nodiscard]] QVariant data(int column) const;
    [[nodiscard]] int row() const;
    [[nodiscard]] TreeItem* parentItem();

private:
    QVector<TreeItem*> _children;
    QVector<QVariant> _data;
    TreeItem* _parent = nullptr;
};
