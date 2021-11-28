#pragma once

#include <QObject>
#include <QList>
#include <QMap>
#include <memory>
#include <optional>

#include "../settings.h"
#include "qschematic_export.h"

namespace QSchematic
{
    class Connector;
}

using namespace QSchematic;

namespace wire_system
{

class net;
class wire;
class connectable;

class QSCHEMATIC_EXPORT manager :
    public QObject
{
    Q_OBJECT

public:
    // Construction
    manager();
    manager(const manager& other) = delete;
    manager(manager&& other) = delete;
    ~manager() override = default;

    // Operators
    manager& operator=(const manager& rhs) = delete;
    manager& operator=(manager&& rhs) = delete;

    void add_net(const std::shared_ptr<net> wireNet);
    [[nodiscard]] QList<std::shared_ptr<net>> nets() const;
    [[nodiscard]] QList<std::shared_ptr<wire>> wires() const;
    void generate_junctions();
    void connect_wire(wire* wire, wire_system::wire* rawWire, std::size_t point);
    void remove_net(std::shared_ptr<net> net);
    void clear();
    bool remove_wire(const std::shared_ptr<wire> wire);
    [[nodiscard]] QVector<std::shared_ptr<wire>> wires_connected_to(const std::shared_ptr<wire>& wire) const;
    void disconnect_wire(const std::shared_ptr<wire_system::wire>& wire, wire_system::wire* otherWire);
    bool add_wire(const std::shared_ptr<wire>& wire);
    void attach_wire_to_connector(wire* wire, int index, const connectable* connector);
    void attach_wire_to_connector(wire* wire, const connectable* connector);
    [[nodiscard]] wire* attached_wire(const connectable* connector);
    [[nodiscard]] int attached_point(const connectable* connector);
    void detach_wire(const connectable* connector);
    [[nodiscard]] std::shared_ptr<wire> wire_with_extremity_at(const QPointF& point);
    void point_inserted(const wire* wire, int index);
    [[nodiscard]] bool point_is_attached(wire_system::wire* wire, int index);
    void set_settings(const Settings& settings);
    [[nodiscard]] Settings settings() const;
    void point_removed(const wire* wire, int index);
    void point_moved_by_user(wire& rawWire, int index);
    void set_net_factory(std::function<std::shared_ptr<net>()> func);
    void connector_moved(const connectable* connector);

signals:
    void wire_point_moved(wire& wire, int index);

private:
    [[nodiscard]] static bool merge_nets(std::shared_ptr<wire_system::net>& net, std::shared_ptr<wire_system::net>& otherNet);

    void detach_wire_from_all(const wire* wire);
    [[nodiscard]] std::shared_ptr<net> create_net();

    QList<std::shared_ptr<net>> m_nets;
    Settings m_settings;
    QMap<const connectable*, QPair<wire*, int>> m_connections;
    std::optional<std::function<std::shared_ptr<net>()>> m_net_factory;
};

}
