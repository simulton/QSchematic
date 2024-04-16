#pragma once

#include <QWidget>

namespace QSchematic::Items
{
    class Item;
}

namespace Library
{

    class Model;
    class View;

    class Widget :
        public QWidget
    {
        Q_OBJECT
        Q_DISABLE_COPY_MOVE(Widget)

    public:
        explicit Widget(QWidget* parent = nullptr);
        ~Widget() override = default;

        void expandAll();

    Q_SIGNALS:
        void itemClicked(const QSchematic::Items::Item* item);

    public Q_SLOTS:
        void setPixmapScale(qreal scale);

    private Q_SLOTS:
        void itemClickedSlot(const QModelIndex& index);

    private:
        Model* _model = nullptr;
        View* _view   = nullptr;
    };

}
