#include "net.h"
#include "wire.h"

#include <QString>

using namespace wire_system;

net::net() : m_manager(nullptr)
{

}

void net::set_name(const std::string& name)
{
    set_name(QString::fromStdString(name));
}

void net::set_name(const QString& name)
{
    m_name = name;
}

QString net::name() const
{
    return m_name;
}

QList<std::shared_ptr<wire>> net::wires() const
{
    QList<std::shared_ptr<wire>> list;

    for (const auto& wire: m_wires) {
        list.append(wire.lock());
    }

    return list;
}

bool net::addWire(const std::shared_ptr<wire>& wire)
{
    // Sanity check
    if (!wire) {
        return false;
    }

    // Update the junctions of the wires that are already in the net
    for (const auto& otherWire : wire->connected_wires()) {
        for (int index = 0; index < otherWire->points_count(); index++) {
            // Ignore if it's not the first/last point
            if (index != 0 && index != otherWire->points_count() - 1) {
                continue;
            }
            // Mark the point as junction if it's on the wire
            if (wire->point_is_on_wire(otherWire->points().at(index).toPointF())) {
                otherWire->set_point_is_junction(index, true);
            }
        }
    }

    wire->setNet(shared_from_this());
    wire->set_manager(manager());

    // Add the wire
    m_wires.append(wire);

    return true;
}

bool net::removeWire(const std::shared_ptr<wire> wire)
{
    for (auto it = m_wires.begin(); it != m_wires.end(); it++) {
        if ((*it).lock() == wire) {
            m_wires.erase(it);
            break;
        }
    }

    return true;
}

bool net::contains(const std::shared_ptr<wire>& wire) const
{
    for (const auto& w : m_wires) {
        if (w.lock() == wire) {
            return true;
        }
    }

    return false;
}

void net::set_manager(class manager* manager)
{
    m_manager = manager;
}

manager* net::manager() const
{
    return m_manager;
}
