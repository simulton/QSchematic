#include "view.hpp"

#include <qschematic/items/item.hpp>
#include <qschematic/items/itemmimedata.hpp>

#include <QDrag>
#include <QPainter>

using namespace Library;

View::View(QWidget* parent) : QTreeView(parent)
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

void View::setPixmapScale(qreal scale)
{
    _scale = scale;
}

void View::startDrag(Qt::DropActions supportedActions)
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
    QSchematic::Items::MimeData* m = qobject_cast<QSchematic::Items::MimeData*>(data);
    if (!m) {
        return;
    }

    // Create the drag object
    QDrag* drag = new QDrag(this);
    QPointF hotSpot;
    drag->setMimeData(data);
    drag->setPixmap(m->item()->toPixmap(hotSpot, _scale));
    drag->setHotSpot(hotSpot.toPoint());

    // Execute the drag
    drag->exec(supportedActions, Qt::CopyAction);
}
