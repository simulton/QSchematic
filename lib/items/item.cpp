#include <QObject>
#include <QPainter>
#include <QVector2D>
#include <QGraphicsSceneHoverEvent>
#include <QTimer>
#include <QWidget>
#include "item.h"
#include "../scene.h"
#include "../commands/commanditemmove.h"

#define BOOL2STR(x) (x ? QStringLiteral("true") : QStringLiteral("false"))
#define STR2BOOL(x) (QString::compare(x, QStringLiteral("true")) == 0 ? true : false)

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
    connect(_hoverTimer, &QTimer::timeout, [this]{
        emit showPopup(*this);
    });

    // Misc
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    connect(this, &Item::xChanged, this, &Item::posChanged);
    connect(this, &Item::yChanged, this, &Item::posChanged);
}

bool Item::toXml(QXmlStreamWriter& xml) const
{
    xml.writeTextElement(QStringLiteral("x"), QString::number(posX()));
    xml.writeTextElement(QStringLiteral("y"), QString::number(posY()));
    xml.writeTextElement(QStringLiteral("movable"), BOOL2STR(isMovable()));
    xml.writeTextElement(QStringLiteral("visible"), BOOL2STR(isVisible()));
    xml.writeTextElement(QStringLiteral("snap_to_grid"), BOOL2STR(snapToGrid()));
    xml.writeTextElement(QStringLiteral("highlight"), BOOL2STR(highlightEnabled()));

    return true;
}

bool Item::fromXml(QXmlStreamReader& reader)
{
    while (reader.readNextStartElement()) {
        if (reader.name() == "x") {
            setPosX(reader.readElementText().toInt());
        } else if (reader.name() == "y"){
            setPosY(reader.readElementText().toInt());
        } else if (reader.name() == "movable") {
            setMovable(STR2BOOL(reader.readElementText()));
        } else if (reader.name() == "visible") {
            setVisible(STR2BOOL(reader.readElementText()));
        } else if (reader.name() == "snap_to_grid") {
            setSnapToGrid(STR2BOOL(reader.readElementText()));
        } else if (reader.name() == "highlight") {
            setHighlightEnabled(STR2BOOL(reader.readElementText()));
        }
    }

    return true;
}

void Item::copyAttributes(Item& dest) const
{
    // Base class
    dest.setParentItem(parentItem());
    dest.setPos(pos());
    dest.setVisible(isVisible());

    // Attributes
    dest._snapToGrid = _snapToGrid;
    dest._highlightEnabled = _highlightEnabled;
    dest._highlighted = _highlighted;
    dest._oldPos = _oldPos;
}

Scene* Item::scene() const
{
    return qobject_cast<Scene*>(QGraphicsObject::scene());
}

void Item::addTypeIdentifierToXml(QXmlStreamWriter& xml) const
{
    xml.writeAttribute(JSON_ID_STRING, QString::number(type()));
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
    setPos(_settings.snapToGrid(pos()));

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

QPixmap Item::toPixmap(QPointF& hotSpot, qreal scale)
{
    // Retrieve the bounding rect
    QRectF rectF = boundingRect();
    rectF = rectF.united(childrenBoundingRect());

    // Adjust the rectangle as the QPixmap doesn't handle negative coordinates
    rectF.setWidth(rectF.width() - rectF.x());
    rectF.setHeight(rectF.height() - rectF.y());
    const QRect& rect = rectF.toRect();
    if (rect.isNull() or !rect.isValid()) {
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
        painter.save();
        painter.translate(child->pos());
        child->paint(&painter, nullptr, nullptr);
        painter.restore();
    }

    painter.end();

    return pixmap;
}

void Item::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)

    setHighlighted(true);
    emit highlightChanged(*this, true);

    update();
}

void Item::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)

    _hoverTimer->start(_settings.popupDelay);
}

void Item::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)

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
            newPos =_settings.snapToGrid(newPos);
        }
        return newPos;
    }

    default:
        return QGraphicsItem::itemChange(change, value);
    }
}

void Item::posChanged()
{
    const QPointF& newPos = pos();
    QVector2D movedBy(newPos - _oldPos);
    if (!movedBy.isNull()) {
        emit moved(*this, movedBy);
    }

    _oldPos = newPos;
}

void Item::update()
{
    // All transformations happen around the center of the item
    setTransformOriginPoint(boundingRect().width()/2, boundingRect().height()/2);

    // Base class
    QGraphicsObject::update();
}

std::unique_ptr<QWidget> Item::popupInfobox() const
{
    return nullptr;
}
