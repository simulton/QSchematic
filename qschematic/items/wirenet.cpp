#include <utils.h>
#include <QVector2D>
#include "connector.h"
#include "../scene.h"
#include "wirenet.h"
#include "wire.h"
#include "label.h"
#include "itemfactory.h"

using namespace QSchematic;

WireNet::WireNet(QObject* parent) :
    QObject(parent), _scene(nullptr)
{
    // Label
    _label = std::make_shared<Label>();
    _label->setPos(0, 0);
    _label->setVisible(false);
    connect(_label.get(), &Label::highlightChanged, this, &WireNet::labelHighlightChanged);
    connect(_label.get(), &Label::moved, this, [=] { updateLabelPos(); });
}

WireNet::~WireNet()
{
    if (_label)
        dissociate_item(_label);
}

gpds::container WireNet::to_container() const
{
    // Wires
    gpds::container wiresContainer;
    for (const auto& wire : _wires) {
        wiresContainer.add_value("wire", wire.lock()->to_container());
    }

    // Root
    gpds::container root;
    root.add_value("name", _name.toStdString() );
    // The coordinates of the label need to be in the scene space
    if (_label->parentItem()) {
        _label->moveBy(QVector2D(_label->parentItem()->pos()));
    }
    root.add_value("label", _label->to_container());
    // Move the label back to the correct position
    if (_label->parentItem()) {
        _label->moveBy(-QVector2D(_label->parentItem()->pos()));
    }
    root.add_value("wires", wiresContainer);

    return root;
}

void WireNet::from_container(const gpds::container& container)
{
    Q_ASSERT(_scene);
    // Root
    setName(QString::fromStdString(container.get_value<std::string>("name").value_or("")));

    // Label
    if (auto labelContainer = container.get_value<gpds::container*>("label").value_or(nullptr)) {
        _label->from_container(*labelContainer);
    }

    // Wires
    {
        const gpds::container& wiresContainer = *container.get_value<gpds::container*>("wires").value();
        for (const gpds::container* wireContainer : wiresContainer.get_values<gpds::container*>("wire")) {
            Q_ASSERT(wireContainer);

            auto newWire = ItemFactory::instance().from_container(*wireContainer);
            auto sharedNewWire = std::dynamic_pointer_cast<Wire>( newWire );
            if (!sharedNewWire) {
                continue;
            }
            sharedNewWire->from_container(*wireContainer);
            addWire(sharedNewWire);
            if (not _scene) {
                qCritical("WireNet::from_container(): The scene has not been set.");
                return;
            }
            _scene->addItem(sharedNewWire);
        }
    }
}

bool WireNet::addWire(const std::shared_ptr<Wire>& wire)
{
    // Sanity check
    if (!wire) {
        return false;
    }

    // Attach to connectors
    if (wire->scene()) {
        for (const auto& connector : wire->scene()->connectors()) {
            if (connector->scenePos() == wire->pointsAbsolute().first()) {
                connector->attachWire(wire.get(), 0);
            } else if (connector->scenePos() == wire->pointsAbsolute().last()) {
                connector->attachWire(wire.get(), wire->pointsAbsolute().count() - 1);
            }
        }
    }

    // Update the junctions of the wires that are already in the net
    for (const auto& otherWire : wire->connectedWires()) {
        for (int index = 0; index < otherWire->pointsAbsolute().count(); index++) {
            // Ignore if it's not the first/last point
            if (index != 0 and index != otherWire->pointsAbsolute().count() - 1) {
                continue;
            }
            // Mark the point as junction if it's on the wire
            if (wire->pointIsOnWire(otherWire->pointsAbsolute().at(index))) {
                otherWire->setPointIsJunction(index, true);
            }
        }
    }

    wire->setNet(shared_from_this());

    // Add the wire
    connect(wire.get(), &Wire::pointMoved, this, &WireNet::wirePointMoved);
    connect(wire.get(), &Wire::pointMovedByUser, this, &WireNet::wirePointMovedByUser);
    connect(wire.get(), &Wire::highlightChanged, this, &WireNet::wireHighlightChanged);
    connect(wire.get(), &Wire::toggleLabelRequested, this, &WireNet::toggleLabel);
    connect(wire.get(), &Wire::moved, this, [=] { updateLabelPos(); });
    connect(wire.get(), &Wire::pointRemoved, this, [=] (int index) { emit pointRemoved(*wire, index);});
    _wires.append(wire);
    updateLabelPos(true);

    return true;
}

bool WireNet::removeWire(const std::shared_ptr<Wire> wire)
{
    disconnect(wire.get(), nullptr, this, nullptr);
    for (auto it = _wires.begin(); it != _wires.end(); it++) {
        if ((*it).lock() == wire) {
            _wires.erase(it);
            break;
        }
    }
    updateLabelPos(true);

    return true;
}

bool WireNet::contains(const std::shared_ptr<Wire>& wire) const
{
    for (const auto& w : _wires) {
        if (w.lock() == wire) {
            return true;
        }
    }

    return false;
}

void WireNet::simplify()
{
    for (auto& wire : _wires) {
        wire.lock()->simplify();
    }
}

void WireNet::setName(const std::string& name)
{
    setName( QString::fromStdString( name ) );
}

void WireNet::setName(const QString& name)
{
    _name = name;

    _label->setText(_name);
    _label->setVisible(!_name.isEmpty());
    updateLabelPos(true);
}

void WireNet::setHighlighted(bool highlighted)
{
    // Wires
    for (auto& wire : _wires) {
        if (wire.expired()) {
            continue;
        }

        wire.lock()->setHighlighted(highlighted);
    }

    // Label
    _label->setHighlighted(highlighted);

    emit highlightChanged(highlighted);
}

void WireNet::setScene(Scene* scene)
{
    _scene = scene;
}

QString WireNet::name()const
{
    return _name;
}

QList<std::shared_ptr<Wire>> WireNet::wires() const
{
    QList<std::shared_ptr<Wire>> list;
    for (const auto& wire: _wires) {
        if (wire.expired()) {
            qDebug() << "expired";
        }
        list.append(wire.lock());
    }
    return list;
}

QList<Line> WireNet::lineSegments() const
{
    QList<Line> list;

    for (const auto& wire : _wires) {
        if (wire.expired()) {
            continue;
        }

        list.append(wire.lock()->lineSegments());
    }

    return list;
}

QList<QPointF> WireNet::points() const
{
    QList<QPointF> list;

    for (const auto& wire : _wires) {
        list.append(wire.lock()->pointsAbsolute().toList());
    }

    return list;
}

std::shared_ptr<Label> WireNet::label()
{
    return _label;
}

void WireNet::wirePointMoved(Wire& wire, WirePoint& point)
{
    updateLabelPos();
    // Let the others know too
    emit pointMoved(wire, point);
}

/**
 * Update the label's connection point and its parent if updateParent is true.
 */
void WireNet::updateLabelPos(bool updateParent) const
{
    // Ignore if the label is not visible
    if (not _label->isVisible()) {
        return;
    }
    // Find closest point
    QPointF labelPos = _label->textRect().center() + _label->scenePos();
    QPointF closestPoint;
    std::shared_ptr<Wire> closestWire;
    for (const auto& wire : _wires) {
        std::shared_ptr<Wire> spWire = wire.lock();
        for (const auto& segment: spWire->lineSegments()) {
            // Find closest point on segment
            QPointF p = Utils::pointOnLineClosestToPoint(segment.p1(), segment.p2(), labelPos);
            float distance1 = QVector2D(labelPos - closestPoint).lengthSquared();
            float distance2 = QVector2D(labelPos - p).lengthSquared();
            if (closestPoint.isNull() or distance1 > distance2) {
                closestPoint = p;
                closestWire = spWire;
            }
        }
    }
    // If there are no wires left in the net it will be hidden anyway
    if (not closestWire) {
        return;
    }
    // Update the parent if requested
    if (updateParent and _label->parentItem() != closestWire.get()) {
        _label->setParentItem(closestWire.get());
        _label->setPos(labelPos - _label->textRect().center() - closestWire->scenePos());
    }
    // Update the connection point
    if (_label->parentItem()) {
        _label->setConnectionPoint(closestPoint - _label->parentItem()->pos());
    } else {
        _label->setConnectionPoint(closestPoint);
    }

}

void WireNet::wirePointMovedByUser(Wire& wire, int index)
{
    emit pointMovedByUser(wire, index);
}

void WireNet::labelHighlightChanged(const Item& item, bool highlighted)
{
    Q_UNUSED(item)

    setHighlighted(highlighted);
}

void WireNet::wireHighlightChanged(const Item& item, bool highlighted)
{
    Q_UNUSED(item)

    setHighlighted(highlighted);
}

void WireNet::toggleLabel()
{
    _label->setVisible(not _label->text().isEmpty() and not _label->isVisible());
    updateLabelPos(true);
}
