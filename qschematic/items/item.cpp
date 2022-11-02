#include "item.h"
#include "../scene.h"
#include "../commands/commanditemmove.h"

#include <QDebug>
#include <QPainter>
#include <QVector2D>
#include <QGraphicsSceneHoverEvent>
#include <QWidget>

using namespace QSchematic;


Item::Item(int type, QGraphicsItem* parent) :
    QGraphicsObject(parent),
    _type(type),
    _snapToGrid(true),
    _highlightEnabled(true),
    _highlighted(false),
    _oldRot{0}
{
    // Misc
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    connect(this, &Item::xChanged, this, &Item::posChanged);
    connect(this, &Item::yChanged, this, &Item::posChanged);
    connect(this, &Item::rotationChanged, this, &Item::rotChanged);

    // Connect signals to parent item
    Item* parentItem = static_cast<Item*>(parent);
    if (parentItem) {
        connect(parentItem, &Item::moved, this, &Item::scenePosChanged);
        connect(parentItem, &Item::rotated, this, &Item::scenePosChanged);
    }
}

Item::~Item()
{
    // Important insurance — if this is NOT expired, then we've been deleted
    // wrongly by Qt and all kinds of dirty shit will hit the fan!
    // Q_ASSERT(SharedPtrTracker::assert_expired(this));
    Q_ASSERT(weakPtr().expired());
}

gpds::container Item::to_container() const
{
    // Root
    gpds::container root;
    addItemTypeIdToContainer(root);
    root.add_value("x", posX());
    root.add_value("y", posY());
    root.add_value("rotation", rotation()).add_attribute("unit", "degrees").add_attribute("direction", "cw");
    root.add_value("movable", isMovable());
    root.add_value("visible", isVisible());
    root.add_value("snap_to_grid", snapToGrid());
    root.add_value("highlight", highlightEnabled());

    return root;
}

void Item::from_container(const gpds::container& container)
{
    setPosX(container.get_value<double>("x").value_or(0));
    setPosY(container.get_value<double>("y").value_or(0));
    setRotation(container.get_value<double>("rotation").value_or(0));
    setMovable(container.get_value<bool>("movable").value_or(true));
    setVisible(container.get_value<bool>("visible").value_or(true));
    setSnapToGrid(container.get_value<bool>("snap_to_grid").value_or(true));
    setHighlightEnabled(container.get_value<bool>("highlight").value_or(false));
}

void Item::copyAttributes(Item& dest) const
{
    // Base class
    dest.setParentItem(parentItem());
    dest.setPos(pos());
    dest.setRotation(rotation());
    dest.setMovable(isMovable());
    dest.setVisible(isVisible());

    // Attributes
    dest._snapToGrid = _snapToGrid;
    dest._highlightEnabled = _highlightEnabled;
    dest._highlighted = _highlighted;
    dest._oldPos = _oldPos;
    dest._oldRot = _oldRot;
}

void Item::addItemTypeIdToContainer(gpds::container& container) const
{
    container.add_attribute( "type_id", type() );
}

Scene* Item::scene() const
{
    return qobject_cast<Scene*>(QGraphicsObject::scene());
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
    if (snapToGrid()) {
        setPos(_settings.snapToGrid(pos()));
    }

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
    return ( ( _highlighted || isSelected() ) && _highlightEnabled );
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
    _highlighted = false;
}

bool Item::highlightEnabled() const
{
    return _highlightEnabled;
}

QPixmap Item::toPixmap(QPointF& hotSpot, qreal scale)
{
    // Retrieve the bounding rect
    QRectF rectF = boundingRect();
    rectF = rectF.united(childrenBoundingRect());

    // Adjust the rectangle as the QPixmap doesn't handle negative coordinates
    rectF.setWidth(rectF.width() - rectF.x());
    rectF.setHeight(rectF.height() - rectF.y());
    const QRect& rect = rectF.toRect();
    if (rect.isNull() || !rect.isValid()) {
        return QPixmap();
    }

    // Provide the hot spot
    hotSpot = -rectF.topLeft();

    // Create the pixmap
    QPixmap pixmap(rect.size() * scale);
    pixmap.fill(Qt::transparent);

    // Render
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, _settings.antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing, _settings.antialiasing);
    painter.scale(scale, scale);
    painter.translate(hotSpot);
    paint(&painter, nullptr, nullptr);
    for (QGraphicsItem* child : childItems()) {
        if (!child)
            continue;
        
        painter.save();
        painter.translate(child->pos());
        child->paint(&painter, nullptr, nullptr);
        painter.restore();
    }

    painter.end();

    return pixmap;
}

QVariant Item::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    switch (change)
    {
    case QGraphicsItem::ItemSceneChange:
    {
        // NOTE: by doing this non-updated scene (ghost images staying) disappeared for some further cases (tips from usenet)
        prepareGeometryChange();
        return QGraphicsItem::itemChange(change, value);
    }
    case QGraphicsItem::ItemPositionChange:
    {
        QPointF newPos = value.toPointF();
        if (snapToGrid()) {
            newPos =_settings.snapToGrid(newPos);
        }
        return newPos;
    }
    case QGraphicsItem::ItemParentChange:
        if (parentObject()) {
            disconnect(parentObject(), nullptr, this, nullptr);
        }
        return value;
    case QGraphicsItem::ItemParentHasChanged:
    {
        Item* parent = static_cast<Item*>(parentItem());
        if (parent) {
            connect(parent, &Item::moved, this, &Item::scenePosChanged);
            connect(parent, &Item::rotated, this, &Item::scenePosChanged);
        }
        return value;
    }

    default:
        return QGraphicsItem::itemChange(change, value);
    }
}

void Item::posChanged()
{
    scenePosChanged();
    const QPointF& newPos = pos();
    QVector2D movedBy(newPos - _oldPos);
    if (!movedBy.isNull()) {
        emit moved(*this, movedBy);
    }

    _oldPos = newPos;
}

void Item::scenePosChanged()
{
    emit movedInScene(*this);
}

void Item::rotChanged()
{
    const qreal newRot = rotation();
    qreal rotationChange = newRot - _oldRot;
    if (!qFuzzyIsNull(rotationChange)) {
        emit rotated(*this, rotationChange);
    }

    _oldRot = newRot;
}

void Item::update()
{
    // All transformations happen around the center of the item
    setTransformOriginPoint(boundingRect().width()/2, boundingRect().height()/2);

    // Base class
    QGraphicsObject::update();
}

