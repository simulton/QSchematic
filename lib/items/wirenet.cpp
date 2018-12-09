#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
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

WireNet::~WireNet()
{
    qDeleteAll(_wires);
}

QJsonObject WireNet::toJson() const
{
    QJsonObject object;

    object.insert("name", name());

    QJsonArray wiresArray;
    for (const Wire* wire : _wires) {
        wiresArray.append(wire->toJson());
    }
    object.insert("wires", wiresArray);

    return object;
}

bool WireNet::fromJson(const QJsonObject& object)
{
    setName(object["name"].toString());

    QJsonArray wiresArray = object["wires"].toArray();
    for (const QJsonValue& wireValue : wiresArray) {
        QJsonObject wireObject = wireValue.toObject();
        if (wireObject.isEmpty())
            continue;
        Wire* newWire = dynamic_cast<Wire*>(ItemFactory::instance().fromJson(wireObject).release());
        if (!newWire)
            continue;
        newWire->fromJson(wireObject);
        addWire(*newWire);
    }

    connect(_label.get(), &Label::highlightChanged, this, &WireNet::labelHighlightChanged);

    return true;
}

bool WireNet::addWire(Wire& wire)
{
    // Update the junctions
    // Do this before we add the wire so that lineSegments() doesn't contain the line segments
    // of the new wire. Otherwise all points will be marked as junctions.
    for (int i = 0; i < wire.points().count(); i++) {
        const WirePoint& point = wire.points().at(i);
        for (const Line& line : lineSegments()) {
            if (line.containsPoint(point.toPoint(), 0)) {
                wire.setPointIsJunction(i, true);
                break;
            }
            wire.setPointIsJunction(i, false);
        }
    }

    // Add the wire
    connect(&wire, &Wire::pointMoved, this, &WireNet::wirePointMoved);
    connect(&wire, &Wire::highlightChanged, this, &WireNet::wireHighlightChanged);
    _wires.append(&wire);

    return true;
}

bool WireNet::removeWire(Wire& wire)
{
    disconnect(&wire, nullptr, this, nullptr);
    _wires.removeAll(&wire);

    updateWireJunctions();

    return true;
}

bool WireNet::contains(const Wire& wire) const
{
    for (const Wire* w : _wires) {
        if (w == &wire) {
            return true;
        }
    }

    return false;
}

void WireNet::simplify()
{
    for (Wire* wire : _wires) {
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
    for (Wire* wire : _wires) {
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

QList<Wire*> WireNet::wires() const
{
    return _wires;
}

QList<Line> WireNet::lineSegments() const
{
    QList<Line> list;

    for (const Wire* wire : _wires) {
        if (!wire) {
            continue;
        }

        list.append(wire->lineSegments());
    }

    return list;
}

QList<QPoint> WireNet::points() const
{
    QList<QPoint> list;

    for (const Wire* wire : _wires) {
        list.append(wire->points().toList());
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
    for (Wire* wire : _wires) {

        // Create a list of wire segments which dont't contains the current wire
        QList<Line> lineSegments;
        for (const Wire* w : _wires) {
            if (w != wire) {
                lineSegments.append(w->lineSegments());
            }
        }

        // Check for each point whether it's part of a line segment
        for (int i = 0; i < wire->points().count(); i++) {
            const WirePoint& point = wire->points().at(i);
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
