#include <QBoxLayout>

#include "itemsslibrarywidget.h"
#include "itemslibrarymodel.h"
#include "itemslibraryview.h"

ItemsLibraryWidget::ItemsLibraryWidget(QWidget* parent) : QWidget(parent)
{
    // Model
    _model = new ItemsLibraryModel(this);

    // View
    _view = new ItemsLibraryView(this);
    _view->setModel(_model);
    connect(_view, &ItemsLibraryView::clicked, this, &ItemsLibraryWidget::itemClickedSlot);

    // Main layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_view);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Misc
    _view->expandAll();
}

void ItemsLibraryWidget::setPixmapScale(qreal scale)
{
    _view->setPixmapScale(scale);
}

void ItemsLibraryWidget::itemClickedSlot(const QModelIndex& index)
{
    // Sanity check
    if (!index.isValid()) {
        return;
    }

    const auto item = _model->itemFromIndex(index);
    if (!item) {
        return;
    }

    emit itemClicked(item);
}

void ItemsLibraryWidget::expandAll()
{
    _view->expandAll();
}
