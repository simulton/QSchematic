#include <QBoxLayout>

#include "widget.hpp"
#include "model.hpp"
#include "view.hpp"

using namespace Library;

Widget::Widget(QWidget* parent) : QWidget(parent)
{
    // Model
    _model = new Model(this);

    // View
    _view = new View(this);
    _view->setModel(_model);
    connect(_view, &View::clicked, this, &Widget::itemClickedSlot);

    // Main layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_view);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Misc
    _view->expandAll();
}

void Widget::setPixmapScale(qreal scale)
{
    _view->setPixmapScale(scale);
}

void Widget::itemClickedSlot(const QModelIndex& index)
{
    // Sanity check
    if (!index.isValid()) {
        return;
    }

    const auto item = _model->itemFromIndex(index);
    if (!item) {
        return;
    }

    Q_EMIT itemClicked(item);
}

void Widget::expandAll()
{
    _view->expandAll();
}
