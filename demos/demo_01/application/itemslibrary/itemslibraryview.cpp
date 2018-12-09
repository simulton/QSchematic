#include <QDrag>
#include <QPainter>
#include "itemslibraryview.h"
#include "../../../lib/items/item.h"
#include "../../../lib/items/itemmimedata.h"

ItemsLibraryView::ItemsLibraryView(QWidget* parent) : QTreeView(parent)
{
    // Initialization
    _scale = 1.0;

    // Configuration
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHeaderHidden(true);
    setIconSize(QSize(28, 28));
}

void ItemsLibraryView::setPixmapScale(qreal scale)
{
    _scale = scale;
}

void ItemsLibraryView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.count() != 1) {
        return;
    }

    // Get a list of the supported MIMEs of the selected indexes
    QMimeData* data = model()->mimeData(indexes);
    if (!data) {
        return;
    }

    // Retrieve the ItemMimeData to get the pixmap
    QSchematic::ItemMimeData* m = qobject_cast<QSchematic::ItemMimeData*>(data);
    if (!m) {
        return;
    }

    // Create the drag object
    QDrag* drag = new QDrag(this);
    drag->setMimeData(data);
    drag->setPixmap(m->item()->toPixmap(_scale));
    drag->setHotSpot(QPoint(0, 0));

    // Execute the drag
    drag->exec(supportedActions, Qt::CopyAction);
}
