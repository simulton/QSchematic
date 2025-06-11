#include "line.hpp"
#include "net.hpp"
#include "wire.hpp"

#include <QString>

using namespace wire_system;

void
net::set_name(const std::string& name)
{
    set_name(QString::fromStdString(name));
}

void
net::set_name(const QString& name)
{
    m_name = name;
}

QString
net::name() const
{
    return m_name;
}

std::vector<std::shared_ptr<wire>>
net::wires() const
{
    std::vector<std::shared_ptr<wire>> list;

    for (const auto& wire: m_wires)
        list.push_back(wire.lock());

    return list;
}

std::vector<point>
net::points() const
{
    std::vector<point> points;

    for (const auto& wire : wires()) {
        #ifdef __cpp_lib_containers_ranges
            points.append_range(wire->points());
        #else
            const auto wps = wire->points();
            points.insert(points.end(), wps.cbegin(), wps.cend());
        #endif
    }

    return points;
}

bool
net::addWire(const std::shared_ptr<wire>& wire)
{
    // Sanity check
    if (!wire)
        return false;

    // Update the junctions of the wires that are already in the net
    for (const auto& otherWire : wire->connected_wires()) {
        for (int index = 0; index < otherWire->points_count(); index++) {
            // Ignore if it's not the first/last point
            if (index != 0 && index != otherWire->points_count() - 1)
                continue;

            // Mark the point as junction if it's on the wire
            if (wire->point_is_on_wire(otherWire->points().at(index).toPointF()))
                otherWire->set_point_is_junction(index, true);
        }
    }

    wire->setNet(shared_from_this());
    wire->set_manager(manager());

    // Add the wire
    m_wires.push_back(wire);

    return true;
}

bool
net::removeWire(const std::shared_ptr<wire> wire)
{
    for (auto it = m_wires.begin(); it != m_wires.end(); it++) {
        if ((*it).lock() == wire) {
            m_wires.erase(it);
            break;
        }
    }

    return true;
}

bool
net::contains(const std::shared_ptr<wire>& wire) const
{
    for (const auto& w : m_wires) {
        if (w.lock() == wire)
            return true;
    }

    return false;
}

std::vector<line>
net::line_segments() const
{
    std::vector<line> list;

    for (const auto& wire : m_wires) {
        auto w = wire.lock();
        if (!w) [[unlikely]]
            continue;

        #ifdef __cpp_lib_containers_ranges
            list.append_range(w->line_segments());
        #else
            auto ls = w->line_segments();
            list.insert(list.end(), ls.cbegin(), ls.cend());
        #endif
    }

    return list;
}

void
net::set_manager(class manager* manager)
{
    m_manager = manager;
}
