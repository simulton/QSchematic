#include "manager.hpp"
#include "net.hpp"
#include "point.hpp"
#include "wire.hpp"
#include "connectable.hpp"

#include <QVector2D>

#include <ranges>

using namespace wire_system;

void
manager::add_net(const std::shared_ptr<net> wireNet)
{
    // Sanity check
    if (!wireNet)
        return;

    wireNet->set_manager(this);

    // Keep track of stuff
    m_nets.push_back(std::move(wireNet));
}

/**
 * Returns a list of all the nets
 */
std::vector<std::shared_ptr<net>>
manager::nets() const
{
    return m_nets;
}

/**
 * Returns a list of all the wires
 */
std::vector<std::shared_ptr<wire>>
manager::wires() const
{
    std::vector<std::shared_ptr<wire>> list;

    for (const auto& wireNet : m_nets) {
        for (const auto& wire : wireNet->wires())
            list.push_back(wire);
    }

    return list;
}

void
manager::generate_junctions()
{
    for (const auto& wire: wires()) {
        for (auto& otherWire: wires()) {
            if (wire == otherWire)
                continue;

            if (wire->point_is_on_wire(otherWire->points().first().toPointF()))
                connect_wire(wire.get(), otherWire.get(), 0);

            if (wire->point_is_on_wire(otherWire->points().last().toPointF()))
                connect_wire(wire.get(), otherWire.get(), otherWire->points().count() - 1);
        }
    }
}

/**
 * Connect a wire to another wire while taking care of merging the nets.
 * @param wire The wire to connect to
 * @param rawWire The wire to connect
 */
void
manager::connect_wire(wire* wire, wire_system::wire* rawWire, std::size_t point)
{
    // Sanity checks
    if (!wire || !rawWire) [[unlikely]]
        return;

    if (!wire->connect_wire(rawWire))
        return;

    std::shared_ptr<wire_system::net> net = wire->net();
    std::shared_ptr<wire_system::net> otherNet = rawWire->net();
    if (merge_nets(net, otherNet))
        remove_net(otherNet);

    // Set the wire point to be a junction
    rawWire->set_point_is_junction(point, true);
}

/**
 * Merges two wirenets into one
 * \param net The net into which the other one will be merged
 * \param otherNet The net to merge into the other one
 * \return Whether the two nets where merged successfully or not
 */
bool
manager::merge_nets(std::shared_ptr<net>& net, std::shared_ptr<wire_system::net>& otherNet)
{
    // Sanity checks
    if (!net || !otherNet) [[unlikely]]
        return false;

    // Ignore if it's the same net
    if (net == otherNet)
        return false;

    for (auto& wire: otherNet->wires()) {
        net->addWire(wire);
        otherNet->removeWire(wire);
    }

    return true;
}

void
manager::remove_net(std::shared_ptr<net> net)
{
    // Sanity check
    if (!net) [[unlikely]]
        return;

    std::erase(m_nets, net);
}

void
manager::clear()
{
    m_nets.clear();
}

void
manager::remove_wire(const std::shared_ptr<wire> wire)
{
    // Sanity check
    if (!wire) [[unlikely]]
        return;

    // Detach from all connectors
    detach_wire_from_all(wire.get());

    // Disconnect from connected wires
    for (const auto& otherWire: wires_connected_to(wire)) {
        if (otherWire != wire) {
            disconnect_wire(otherWire, wire.get());
            // Update the junction on the other wire
            for (int index = 0; index < otherWire->points_count(); index++) {
                const auto point = otherWire->points().at(index);
                if (!point.is_junction())
                    continue;

                if (wire->point_is_on_wire(point.toPointF()))
                    otherWire->set_point_is_junction(index, false);
            }
        }
    }

    // Remove the wire from the list
    QList<std::shared_ptr<net>> netsToDelete;
    for (auto& net : m_nets) {
        if (net->contains(wire))
            net->removeWire(wire);

        if (std::size(net->wires()) < 1)
            netsToDelete.append(net);
    }

    // Delete the net if this was the nets last wire
    for (auto& net : netsToDelete)
        remove_net(net);
}


/**
 * Generates a list of all the wires connected to a certain wire including the
 * wire itself.
 */
std::list<std::shared_ptr<wire>>
manager::wires_connected_to(const std::shared_ptr<wire>& wire) const
{
    // Sanity check
    if (!wire) [[unlikely]]
        return { };

    std::list<std::shared_ptr<wire_system::wire>> connectedWires;

    // Add the wire itself to the list
    connectedWires.push_back(wire);

    std::list<std::shared_ptr<wire_system::wire>> newList;
    do {
        newList.clear();
        // Go through all the wires in the net
        for (const auto& otherWire: wire->net()->wires()) {
            // Ignore if the wire is already in the list
            if (std::ranges::contains(connectedWires, otherWire))
                continue;

            // If they are connected to one of the wire in the list add them to the new list
            for (const auto& wire2 : connectedWires) {
                if (wire2->connected_wires().contains(otherWire.get())) {
                    newList.push_back(otherWire);
                    break;
                }
                if (otherWire->connected_wires().contains(wire2.get())) {
                    newList.push_back(otherWire);
                    break;
                }
            }
        }

        #ifdef __cpp_lib_containers_ranges
            connectedWires.append_range(newList);
        #else
            connectedWires.insert(connectedWires.end(), newList.cbegin(), newList.cend());
        #endif
    } while (!std::empty(newList));

    return connectedWires;
}

/**
 * Disconnects the a wire from another and takes care of updating the wirenets.
 * \param wire The wire that the other is attached to
 * \param otherWire The wire that is being disconnected
 */
void
manager::disconnect_wire(const std::shared_ptr<wire_system::wire>& wire, wire_system::wire* otherWire)
{
    // Sanity checks
    if (!wire || !otherWire) [[unlikely]]
        return;

    wire->disconnectWire(otherWire);
    auto net = otherWire->net();

    // Create a list of wires that will stay in the old net
    std::list<std::shared_ptr<wire_system::wire>> oldWires = wires_connected_to(wire);

    // If there are wires that are not in the list create a new net
    if (std::size(net->wires()) != std::size(oldWires)) {
        // Create new net and add the wire
        auto newNet = create_net();
        add_net(std::static_pointer_cast<wire_system::net>(newNet));
        for (auto wireToMove: net->wires()) {
            if (std::ranges::contains(oldWires, wireToMove))
                continue;

            newNet->addWire(wireToMove);
            net->removeWire(wireToMove);
        }
    }
}

bool
manager::add_wire(const std::shared_ptr<wire>& wire)
{
    // Sanity check
    if (!wire)
        return false;

    wire->set_manager(this);

    // No point of the new wire lies on an existing line segment - create a new wire net
    auto newNet = create_net();
    newNet->addWire(wire);
    add_net(std::static_pointer_cast<wire_system::net>(newNet));

    return true;
}

void
manager::point_moved_by_user(wire& rawWire, int index)
{
    point point = rawWire.points().at(index);

    Q_EMIT wire_point_moved(rawWire, index);

    // Detach wires
    if (index == 0 || index == rawWire.points_count() - 1){
        if (point.is_junction()) {
            for (const auto& wire: wires()) {
                // Skip current wire
                if (wire.get() == &rawWire)
                    continue;

                // If is connected
                if (wire->connected_wires().contains(&rawWire)) {
                    bool shouldDisconnect = true;

                    // Keep the wires connected if there is another junction
                    for (const auto& jIndex : rawWire.junctions()) {
                        const auto& junction = rawWire.points().at(jIndex);
                        // Ignore the point that moved
                        if (jIndex == index)
                            continue;

                        // If the point is on the line stay connected
                        if (wire->point_is_on_wire(junction.toPointF())) {
                            shouldDisconnect = false;
                            break;
                        }
                    }

                    if (shouldDisconnect)
                        disconnect_wire(wire, &rawWire);

                    rawWire.set_point_is_junction(index, false);
                }
            }
        }
    }

    // Attach point to wire if needed
    if (index == 0 || index == rawWire.points().count() - 1) {
        for (const auto& wire: wires()) {
            // Skip current wire
            if (wire.get() == &rawWire)
                continue;

            if (wire->point_is_on_wire(rawWire.points().at(index).toPointF())) {
                if (!rawWire.connected_wires().contains(wire.get()))
                    connect_wire(wire.get(), &rawWire, index);
            }
        }
    }
}

void
manager::attach_wire_to_connector(wire* wire, int index, const connectable* connector)
{
    // Sanity check
    if (!wire || !connector) [[unlikely]]
        return;

    // Boundary check
    if (index < 0 || index >= wire->points().count())
        return;

    // Note: Does nothing if the key already exists
    m_connections.try_emplace(connector, connection_record{wire, index});
}

/**
 * Connects a wire to a connector and finds out with end should be connected.
 * \remark If the connector is not on one of the ends, it does nothing
 */
void
manager::attach_wire_to_connector(wire* wire, const connectable* connector)
{
    // Sanity checks
    if (!wire || !connector) [[unlikely]]
        return;

    // Check if it's the first point
    if (wire->points().first().toPoint() == connector->position().toPoint())
        attach_wire_to_connector(wire, 0, connector);

    // Check if it's the last point
    else if (wire->points().last().toPoint() == connector->position().toPoint())
        attach_wire_to_connector(wire, wire->points().count() - 1, connector);
}

void
manager::point_inserted(const wire* wire, int index)
{
    // Sanity check
    if (!wire) [[unlikely]]
        return;

    // Find the connection record
    auto it = std::ranges::find_if(
        m_connections,
        [wire](const auto& item){
            const auto& cr = item.second;
            return cr.wire == wire;
        }
    );
    if (it == std::cend(m_connections))
        return;

    // ToDo: Can we take an lvalue ref and modify it "in place" without a call to insert_or_assign?
    auto cr = it->second;

    // Do nothing if the connected point is the first
    if (cr.point_index == 0)
        return;

    // Inserted point comes before the connected point or the last point is connected
    else if (cr.point_index >= index || cr.point_index == wire->points_count() - 2)
        cr.point_index++;

    // Update the connection
    m_connections.insert_or_assign(it->first, cr);
}

void
manager::point_removed(const wire* wire, int index)
{
    // Sanity check
    if (!wire) [[unlikely]]
        return;

    // Find the connection record
    auto it = std::ranges::find_if(
        m_connections,
        [wire](const auto& item){
            const auto& cr = item.second;
            return cr.wire == wire;
        }
    );
    if (it == std::cend(m_connections))
        return;

    // ToDo: Can we take an lvalue ref and modify it "in place" without a call to insert_or_assign?
    auto cr = it->second;

    if (cr.point_index >= index)
        cr.point_index--;

    // Update the connection
    m_connections.insert_or_assign(it->first, cr);
}

void
manager::detach_wire(const connectable* connector)
{
    // Sanity check
    if (!connector) [[unlikely]]
        return;

    m_connections.erase(connector);
}

std::shared_ptr<wire>
manager::wire_with_extremity_at(const QPointF& point)
{
    for (const auto& wire : wires()) {
        for (const auto& p : wire->points()) {
            if (p.toPoint() == point.toPoint())
                return wire;
        }
    }

    return nullptr;
}

void
manager::detach_wire_from_all(const wire* wire)
{
    // Sanity check
    if (!wire) [[unlikely]]
        return;

    std::erase_if(
        m_connections,
        [wire](const auto& item) {
            const auto& cr = item.second;

            return cr.wire == wire;
        }
    );
}

wire*
manager::attached_wire(const connectable* connector)
{
    // Sanity check
    if (!connector) [[unlikely]]
        return nullptr;

    const auto cr = attached_wire2(connector);
    if (!cr)
        return nullptr;

    return cr->wire;
}

int
manager::attached_point(const connectable* connector)
{
    // Sanity check
    if (!connector) [[unlikely]]
        return -1;

    const auto cr = attached_wire2(connector);
    if (!cr)
        return -1;

    return cr->point_index;
}

std::optional<manager::connection_record>
manager::attached_wire2(const connectable* connector)
{
    // Sanity check
    if (!connector) [[unlikely]]
        return std::nullopt;

    const auto it = m_connections.find(connector);
    if (it == std::cend(m_connections))
        return std::nullopt;

    return it->second;
}

void
manager::connector_moved(const connectable* connector)
{
    // Sanity check
    if (!connector) [[unlikely]]
        return;

    // Find connection_record
    const auto it = m_connections.find(connector);
    if (it == std::cend(m_connections))
        return;
    const auto cr = it->second;

    // Sanity checks
    if (!cr.wire) [[unlikely]]
        return;
    if (cr.point_index < 0 || cr.point_index >= cr.wire->points_count()) [[unlikely]]
        return;

    QPointF oldPos = cr.wire->points().at(cr.point_index).toPointF();
    QVector2D moveBy = QVector2D(connector->position() - oldPos);
    if (!moveBy.isNull())
        cr.wire->move_point_by(cr.point_index, moveBy);
}

/**
 * Returns whether the wire's point is attached to a connector
 */
bool
manager::point_is_attached(wire_system::wire* wire, int index) const
{
    // Sanity check
    if (!wire) [[unlikely]]
        return false;

    const auto it = std::ranges::find_if(
        m_connections,
        [wire, index](const auto& item) {
            const auto& cr = item.second;
            return cr.wire == wire && cr.point_index == index;
        }
    );

    return it != std::cend(m_connections);
}

void
manager::set_settings(const Settings& settings)
{
    m_settings = settings;
}

Settings
manager::settings() const
{
    return m_settings;
}

void
manager::set_net_factory(std::function<std::shared_ptr<net>()> func)
{
    m_net_factory = std::move(func);
}

std::shared_ptr<net>
manager::create_net()
{
    std::shared_ptr<net> net;
    if (m_net_factory)
        net = m_net_factory();
    else
        net = std::make_shared<class net>();

    net->set_manager(this);

    return net;
}
