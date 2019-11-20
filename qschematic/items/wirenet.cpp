#include <utils.h>
#include <QVector2D>
#include "../scene.h"
#include "wirenet.h"
#include "wire.h"
#include "label.h"
#include "itemfactory.h"

using namespace QSchematic;

WireNet::WireNet(QObject* parent) :
    QObject(parent)
{
    // Label
    _label = QSchematic::mk_sh<Label>();
    _label->setPos(0, 0);
    _label->setVisible(false);
    connect(_label.get(), &Label::highlightChanged, this, &WireNet::labelHighlightChanged);
    connect(_label.get(), &Label::moved, this, &WireNet::updateLabelPos);
}

WireNet::~WireNet()
{
    dissociate_item(_label);
}

gpds::container WireNet::to_container() const
{
    // Wires
    gpds::container wiresContainer;
    for (const auto& wire : _wires) {
        wiresContainer.add_value("wire", wire->to_container());
    }

    // Root
    gpds::container root;
    root.add_value("name", _name.toStdString() );
    root.add_value("label", _label->to_container());
    root.add_value("wires", wiresContainer);

    return root;
}

void WireNet::from_container(const gpds::container& container)
{
    // Root
    setName( QString::fromStdString( container.get_value<std::string>( "name" ) ) );

    // Label
    // NOTE: I'm not sure if this should be here, is new, or was on the way out — confirm! Missing in my saved test-files... /Oscar
    if ( auto labelContainer = container.get_value<gpds::container*>( "label" ) ) {
        _label->from_container(*labelContainer);
    }

    // Wires
    {
        const gpds::container& wiresContainer = *container.get_value<gpds::container*>( "wires" );
        for (const gpds::container* wireContainer : wiresContainer.get_values<gpds::container*>( "wire" ) ) {
            Q_ASSERT(wireContainer);

            auto newWire = ItemFactory::instance().from_container(*wireContainer);
            auto sharedNewWire = std::dynamic_pointer_cast<Wire>( newWire );
            if (!sharedNewWire) {
                continue;
            }
            sharedNewWire->from_container(*wireContainer);
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

    wire->setNet(shared_from_this());

    // Add the wire
    connect(wire.get(), &Wire::pointMoved, this, &WireNet::wirePointMoved);
    connect(wire.get(), &Wire::pointMovedByUser, this, &WireNet::wirePointMovedByUser);
    connect(wire.get(), &Wire::highlightChanged, this, &WireNet::wireHighlightChanged);
    connect(wire.get(), &Wire::toggleLabelRequested, this, &WireNet::toggleLabel);
    connect(wire.get(), &Wire::moved, this, &WireNet::updateLabelPos);
    _wires.append(wire);
    updateLabelPos();

    return true;
}

bool WireNet::removeWire(const std::shared_ptr<Wire> wire)
{
    disconnect(wire.get(), nullptr, this, nullptr);
    _wires.removeAll(wire);
    updateLabelPos();

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
    updateLabelPos();
    // Let the others know too
    emit pointMoved(wire, point);
}

void WireNet::updateLabelPos() const
{
    QPointF labelPos = _label->textRect().center() + _label->pos();
    QPointF closestPoint;
    for (const auto& segment: lineSegments()) {
        // Find closest point on segment
         QPointF p = Utils::pointOnLineClosestToPoint(segment.p1(), segment.p2(), labelPos);
         float distance1 = QVector2D(labelPos - closestPoint).lengthSquared();
         float distance2 = QVector2D(labelPos - p).lengthSquared();
         if (closestPoint.isNull() or distance1 > distance2) {
              closestPoint = p;
         }
    }
    _label->setConnectionPoint(closestPoint);
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
}
