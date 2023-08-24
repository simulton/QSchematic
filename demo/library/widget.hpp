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

    signals:
        void itemClicked(const QSchematic::Items::Item* item);

    public slots:
        void setPixmapScale(qreal scale);

    private slots:
        void itemClickedSlot(const QModelIndex& index);

    private:
        Model* _model = nullptr;
        View* _view   = nullptr;
    };

}
