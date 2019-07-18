#pragma once

#include <QWidget>

namespace QSchematic {
    class Item;
}

class ItemsLibraryModel;
class ItemsLibraryView;

class ItemsLibraryWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ItemsLibraryWidget)

public:
    explicit ItemsLibraryWidget(QWidget* parent = nullptr);
    virtual ~ItemsLibraryWidget() override = default;

    void expandAll();

signals:
    void itemClicked(const QSchematic::Item* item);

public slots:
    void setPixmapScale(qreal scale);

private slots:
    void itemClickedSlot(const QModelIndex& index);

private:
    ItemsLibraryModel* _model;
    ItemsLibraryView* _view;
};
