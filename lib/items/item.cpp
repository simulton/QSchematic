#include <QObject>
#include <QPainter>
#include <QVector2D>
#include <QGraphicsSceneHoverEvent>
#include <QTimer>
#include <QJsonObject>
#include "item.h"
#include "../scene.h"
#include "../commands/commanditemmove.h"

using namespace QSchematic;

Item::Item(int type, QGraphicsItem* parent) :
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

QJsonObject Item::toJson() const
{
    QJsonObject object;

    object.insert("pos x", posX());
    object.insert("pos y", posY());
    object.insert("movable", isMovable());
    object.insert("visible", isVisible());
    object.insert("snap to grid", snapToGrid());
    object.insert("highlight enabled", highlightEnabled());

    return object;
}

bool Item::fromJson(const QJsonObject& object)
{
    setPosX(object["pos x"].toDouble());
    setPosY(object["pos y"].toDouble());
    setMovable(object["movable"].toBool());
    setVisible(object["visible"].toBool());
    setSnapToGrid(object["snap to grid"].toBool());
    setHighlightEnabled(object["highlight enabled"].toBool());

    return true;
}

void Item::copyAttributes(Item& dest) const
{
    dest.setParentItem(parentItem());

    dest._snapToGrid = _snapToGrid;
    dest._highlightEnabled = _highlightEnabled;
    dest._highlighted = _highlighted;
    dest._oldGridPoint = _oldGridPoint;
}

Scene* Item::scene() const
{
    return qobject_cast<Scene*>(QGraphicsObject::scene());
}

void Item::addTypeIdentifierToJson(QJsonObject& object) const
{
    object.insert(JSON_ID_STRING, type());
}

int Item::type() const
{
    return _type;
}

void Item::setGridPos(const QPoint& gridPos)
{
    setPos(_settings.toScenePoint(gridPos));
}

void Item::setGridPos(int x, int y)
{
    setGridPos(QPoint(x, y));
}

void Item::setGridPosX(int x)
{
    setGridPos(x, gridPosY());
}

void Item::setGridPosY(int y)
{
    setGridPos(gridPosX(), y);
}

QPoint Item::gridPos() const
{
    return _settings.toGridPoint(pos().toPoint());
}

int Item::gridPosX() const
{
    return gridPos().x();
}

int Item::gridPosY() const
{
    return gridPos().y();
}

void Item::setPos(const QPointF& pos)
{
    QGraphicsObject::setPos(pos);
}

void Item::setPos(qreal x, qreal y)
{
    QGraphicsObject::setPos(x, y);
}

void Item::setPosX(qreal x)
{
    setPos(x, posY());
}

void Item::setPosY(qreal y)
{
    setPos(posX(), y);
}

QPointF Item::pos() const
{
    return QGraphicsObject::pos();
}

qreal Item::posX() const
{
    return pos().x();
}

qreal Item::posY() const
{
    return pos().y();
}

void Item::setScenePos(const QPointF& point)
{
    QGraphicsObject::setPos(mapToParent(mapFromScene(point)));
}

void Item::setScenePos(qreal x, qreal y)
{
    setScenePos(QPointF(x, y));
}

void Item::setScenePosX(qreal x)
{
    setScenePos(x, scenePosY());
}

void Item::setScenePosY(qreal y)
{
    setScenePos(scenePosX(), y);
}

QPointF Item::scenePos() const
{
    return QGraphicsObject::scenePos();
}

qreal Item::scenePosX() const
{
    return scenePos().x();
}

qreal Item::scenePosY() const
{
    return scenePos().y();
}

void Item::moveBy(const QVector2D& moveBy)
{
    setPos(pos() + moveBy.toPointF());
}

void Item::setSettings(const Settings& settings)
{
    // Resnap to grid
    setPos(_settings.snapToGridPoint(pos()));

    // Store the new settings
    _settings = settings;

    // Let everyone know
    emit settingsChanged();

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

bool Item::highlightEnabled() const
{
    return _highlightEnabled;
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
    painter.setRenderHint(QPainter::Antialiasing, _settings.antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing, _settings.antialiasing);
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
    switch (change)
    {
    case QGraphicsItem::ItemPositionChange:
    {
        QPointF newPos = value.toPointF();
        if (snapToGrid()) {
            newPos =_settings.snapToGridPoint(newPos);
        }
        return newPos;
    }

    default:
        return QGraphicsItem::itemChange(change, value);
    }
}

void Item::timerTimeout()
{
    emit showPopup(*this);
}

void Item::posChanged()
{
    QPoint newGridPoint = gridPos();
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
