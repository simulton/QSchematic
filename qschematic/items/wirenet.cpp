#include "wirenet.h"
#include "wire.h"
#include "label.h"
#include "itemfactory.h"

using namespace QSchematic;

WireNet::WireNet(QObject* parent) :
    QObject(parent)
{
    // Label
    _label = QSchematic::make_origin<Label>();
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
    root.addValue("name", std::move( _name.toStdString() ) );
    root.addValue("wires", wiresContainer);

    return root;
}

void WireNet::fromContainer(const Gpds::Container& container)
{
    // Root
    setName( QString::fromStdString( container.getValue<std::string>( "name" ) ) );

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

    // Add the wire
    connect(wire.get(), &Wire::pointMoved, this, &WireNet::wirePointMoved);
    connect(wire.get(), &Wire::pointMovedByUser, this, &WireNet::wirePointMovedByUser);
    connect(wire.get(), &Wire::highlightChanged, this, &WireNet::wireHighlightChanged);
    _wires.append(wire);

    return true;
}

bool WireNet::removeWire(const std::shared_ptr<Wire> wire)
{
    disconnect(wire.get(), nullptr, this, nullptr);
    _wires.removeAll(wire);

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

void WireNet::setName(const std::string& name)
{
    setName( QString::fromStdString( name ) );
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
    // Let the others know too
    emit pointMoved(wire, point);
}

void WireNet::wirePointMovedByUser(Wire& wire, WirePoint& point)
{
    emit pointMovedByUser(wire, point);
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
