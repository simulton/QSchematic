#include <QObject>
#include <QPainter>
#include <QVector2D>
#include <QGraphicsSceneHoverEvent>
#include <QTimer>
#include "item.h"
#include "../scene.h"

using namespace QSchematic;

Item::Item(ItemType type, QGraphicsItem* parent) :
    QGraphicsObject(parent),
    _type(type),
    _snapToGrid(true),
    _highlightEnabled(true),
    _highlighted(false)
{
    // Highlight timer
    _hoverTimer = new QTimer(this);
    _hoverTimer->setSingleShot(true);
    connect(_hoverTimer, &QTimer::timeout, this, &Item::timerTimeout);

    // Misc
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    connect(this, &Item::xChanged, this, &Item::posChanged);
    connect(this, &Item::yChanged, this, &Item::posChanged);
}

int Item::type() const
{
    return _type;
}

void Item::setGridPoint(const QPoint& newGridPoint)
{
    setPos(_settings.toScenePoint(newGridPoint));
}

void Item::setGridPoint(int x, int y)
{
    setGridPoint(QPoint(x, y));
}

void Item::setGridPointX(int x)
{
    setGridPoint(x, gridPointY());
}

void Item::setGridPointY(int y)
{
    setGridPoint(gridPointX(), y);
}

QPoint Item::gridPoint() const
{
    return _settings.toGridPoint(pos().toPoint());
}

int Item::gridPointX() const
{
    return gridPoint().x();
}

int Item::gridPointY() const
{
    return gridPoint().y();
}

QPointF Item::scenePoint() const
{
    return scenePos();
}

qreal Item::scenePointX() const
{
    return scenePoint().x();
}

qreal Item::scenePointY() const
{
    return scenePoint().y();
}

void Item::setSettings(const Settings& settings)
{
    // Update grid size
    setPos((pos() / _settings.gridSize) * settings.gridSize);

    // Store the new settings
    _settings = settings;

    // Update
    update();
}

const Settings& Item::settings() const
{
    return _settings;
}

void Item::setMovable(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsMovable, enabled);
}

bool Item::isMovable() const
{
    return flags() & QGraphicsItem::ItemIsMovable;
}

void Item::setSnapToGrid(bool enabled)
{
    _snapToGrid = enabled;
}

bool Item::snapToGrid() const
{
    return _snapToGrid;
}

bool Item::isHighlighted() const
{
    return (_highlighted || isSelected()) && _highlightEnabled;
}

void Item::setHighlighted(bool highlighted)
{
    _highlighted = highlighted;

    // Ripple through children
    for (QGraphicsItem* child : childItems()) {
        Item* childItem = qgraphicsitem_cast<Item*>(child);
        if (childItem) {
            childItem->setHighlighted(highlighted);
        }
    }
}

void Item::setHighlightEnabled(bool enabled)
{
    _highlightEnabled = enabled;
}

QPixmap Item::toPixmap(qreal scale)
{
    // Retrieve the bounding rect
    QRect rect = boundingRect().toRect();
    if (rect.isNull() || !rect.isValid()) {
        return QPixmap();
    }

    // Create the pixmap
    QPixmap pixmap(rect.size() * scale);
    pixmap.fill(Qt::transparent);

    // Render
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.translate(-rect.topLeft());
    painter.scale(scale, scale);
    paint(&painter, nullptr, nullptr);
    for (QGraphicsItem* child : childItems()) {
        painter.save();
        painter.translate(child->mapToParent(pos()));
        child->paint(&painter, nullptr, nullptr);
        painter.restore();
    }

    painter.end();

    return pixmap;
}

void Item::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    event->accept();

    setHighlighted(true);
    emit highlightChanged(*this, true);

    update();
}

void Item::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    event->accept();

    _hoverTimer->start(_settings.popupDelay);
}

void Item::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    event->accept();

    _hoverTimer->stop();
    setHighlighted(false);
    emit highlightChanged(*this, false);

    update();
}

QVariant Item::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
    case QGraphicsItem::ItemPositionChange:
        if (snapToGrid()) {
            QPoint newPos = _settings.snapToGridPoint(value.toPointF());
            return newPos;
        }
        return value;

    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
}

void Item::timerTimeout()
{
    emit showPopup(*this);
}

void Item::posChanged()
{
    QPoint newGridPoint = gridPoint();
    QVector2D movedBy(newGridPoint - _oldGridPoint);
    if (!movedBy.isNull()) {
        emit moved(*this, movedBy);
    }

    _oldGridPoint = newGridPoint;
}

void Item::update()
{
    // All transformations happen around the center of the item
    setTransformOriginPoint(boundingRect().width()/2, boundingRect().height()/2);

    // Base class
    QGraphicsObject::update();
}

QWidget* Item::popupInfobox() const
{
    return nullptr;
}
