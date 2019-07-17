#include "wirenet.h"
#include "wire.h"
#include "label.h"
#include "itemfactory.h"

using namespace QSchematic;

WireNet::WireNet(QObject* parent) :
    QObject(parent)
{
    // Label
    _label = std::make_shared<Label>();
    _label->setPos(0, 0);
    connect(_label.get(), &Label::highlightChanged, this, &WireNet::labelHighlightChanged);
}

Gpds::Container WireNet::toContainer() const
{
    // Wires
    Gpds::Container wiresContainer;
    for (const auto& wire : _wires) {
        wiresContainer.addValue("wire", wire->toContainer());
    }

    // Root
    Gpds::Container root;
    root.addValue("name", _name);
    root.addValue("wires", wiresContainer);

    return root;
}

void WireNet::fromContainer(const Gpds::Container& container)
{
    // Root
    setName( container.getValue<QString>( "name" ) );

    // Wires
    {
        const Gpds::Container& wiresContainer = *container.getValue<Gpds::Container*>( "wires" );
        for (const Gpds::Container* wireContainer : wiresContainer.getValues<Gpds::Container*>( "wire" ) ) {
            Q_ASSERT(wireContainer);

            auto newWire = ItemFactory::instance().fromContainer(*wireContainer);
            auto sharedNewWire = std::dynamic_pointer_cast<Wire>( std::shared_ptr<Item>( std::move(newWire) ) );
            if (!sharedNewWire) {
                continue;
            }
            sharedNewWire->fromContainer(*wireContainer);
            addWire(sharedNewWire);
        }
    }
}

bool WireNet::addWire(const std::shared_ptr<Wire>& wire)
{
    // Sanity check
    if (!wire) {
        return false;
    }

    // Update the junctions
    // Do this before we add the wire so that lineSegments() doesn't contain the line segments
    // of the new wire. Otherwise all points will be marked as junctions.
    for (int i = 0; i < wire->pointsRelative().count(); i++) {
        const WirePoint& point = wire->pointsRelative().at(i);
        for (const Line& line : lineSegments()) {
            wire->setPointIsJunction(i, false);
            if (line.containsPoint(point.toPoint(), 0)) {
                wire->setPointIsJunction(i, true);
                break;
            }
        }
    }

    // Check if we dropped on a point of the existing wire
    // If so, mark that one as a junction
    for (auto& existingWire : _wires) {
        auto existingWirePointsRelative = existingWire->wirePointsRelative();
        for (int i = 0; i < existingWirePointsRelative.count(); i++) {
            const WirePoint& existingWirePoint = existingWirePointsRelative.at(i);
            for (const auto& wirePoint : wire->pointsRelative()) {
                if (existingWirePoint == wirePoint) {
                    existingWire->setPointIsJunction(i, true);
                }
            }
        }
    }

    // Add the wire
    connect(wire.get(), &Wire::pointMoved, this, &WireNet::wirePointMoved);
    connect(wire.get(), &Wire::highlightChanged, this, &WireNet::wireHighlightChanged);
    _wires.append(wire);

    return true;
}

bool WireNet::removeWire(const std::shared_ptr<Wire>& wire)
{
    disconnect(wire.get(), nullptr, this, nullptr);
    _wires.removeAll(wire);

    updateWireJunctions();

    return true;
}

bool WireNet::contains(const std::shared_ptr<Wire>& wire) const
{
    for (const auto& w : _wires) {
        if (w == wire) {
            return true;
        }
    }

    return false;
}

void WireNet::simplify()
{
    for (auto& wire : _wires) {
        wire->simplify();
    }
}

void WireNet::setName(const QString& name)
{
    _name = name;

    _label->setText(_name);
    _label->setVisible(!_name.isEmpty());
}

void WireNet::setHighlighted(bool highlighted)
{
    // Wires
    for (auto& wire : _wires) {
        if (!wire) {
            continue;
        }

        wire->setHighlighted(highlighted);
    }

    // Label
    _label->setHighlighted(highlighted);

    emit highlightChanged(highlighted);
}

QString WireNet::name()const
{
    return _name;
}

QList<std::shared_ptr<Wire>> WireNet::wires() const
{
    return _wires;
}

QList<Line> WireNet::lineSegments() const
{
    QList<Line> list;

    for (const auto& wire : _wires) {
        if (!wire) {
            continue;
        }

        list.append(wire->lineSegments());
    }

    return list;
}

QList<QPointF> WireNet::points() const
{
    QList<QPointF> list;

    for (const auto& wire : _wires) {
        list.append(wire->pointsAbsolute().toList());
    }

    return list;
}

std::shared_ptr<Label> WireNet::label()
{
    return _label;
}

void WireNet::wirePointMoved(Wire& wire, WirePoint& point)
{
    // Clear the junction
    point.setIsJunction(false);

    // Let the others know too
    emit pointMoved(wire, point);
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

void WireNet::updateWireJunctions()
{
    for (auto& wire : _wires) {

        // Create a list of wire segments which dont't contains the current wire
        QList<Line> lineSegments;
        for (const auto& w : _wires) {
            if (w != wire) {
                lineSegments.append(w->lineSegments());
            }
        }

        // Check for each point whether it's part of a line segment
        for (int i = 0; i < wire->pointsRelative().count(); i++) {
            const WirePoint& point = wire->pointsRelative().at(i);
            for (const Line& line : lineSegments) {
                if (line.containsPoint(point.toPoint(), 0)) {
                    wire->setPointIsJunction(i, true);
                    break;
                }
                wire->setPointIsJunction(i, false);
            }
        }
    }
}
