#pragma once

#include "../settings.hpp"

#include <QObject>
#include <QMap>

#include <list>
#include <memory>
#include <utility>
#include <vector>

namespace QSchematic::Items
{
    class Connector;
}

using namespace QSchematic;

namespace wire_system
{

    class net;
    class wire;
    struct connectable;

    class manager :
        public QObject
    {
        Q_OBJECT

    Q_SIGNALS:
        void wire_point_moved(wire& wire, int index);

    public:
        // Construction
        manager() = default;
        manager(const manager& other) = delete;
        manager(manager&& other) = delete;
        ~manager() override = default;

        // Operators
        manager& operator=(const manager& rhs) = delete;
        manager& operator=(manager&& rhs) = delete;

        void
        add_net(std::shared_ptr<net> wireNet);

        [[nodiscard]]
        std::list<std::shared_ptr<net>>
        nets() const;

        [[nodiscard]]
        std::list<std::shared_ptr<wire>>
        wires() const;

        void
        generate_junctions();

        void
        connect_wire(wire* wire, wire_system::wire* rawWire, std::size_t point);

        void
        remove_net(std::shared_ptr<net> net);

        void
        clear();

        void
        remove_wire(std::shared_ptr<wire> wire);

        [[nodiscard]]
        std::vector<std::shared_ptr<wire>>
        wires_connected_to(const std::shared_ptr<wire>& wire) const;

        void
        disconnect_wire(const std::shared_ptr<wire_system::wire>& wire, wire_system::wire* otherWire);

        bool
        add_wire(const std::shared_ptr<wire>& wire);

        void
        attach_wire_to_connector(wire* wire, int index, const connectable* connector);

        void
        attach_wire_to_connector(wire* wire, const connectable* connector);

        void
        detach_wire(const connectable* connector);

        /**
         * Get wire connected to the specified connector.
         */
        [[nodiscard]]
        std::pair<wire*, int>
        attached_wire(const connectable* connector);

        [[nodiscard]]
        std::shared_ptr<wire>
        wire_with_extremity_at(const QPointF& point);

        void
        point_inserted(const wire* wire, int index);

        [[nodiscard]]
        bool
        point_is_attached(wire_system::wire* wire, int index) const;

        void
        set_settings(const Settings& settings);

        [[nodiscard]]
        Settings
        settings() const;

        void
        point_removed(const wire* wire, int index);

        void
        point_moved_by_user(wire& rawWire, int index);

        void
        set_net_factory(std::function<std::shared_ptr<net>()> func);

        void
        connector_moved(const connectable* connector);

    private:
        std::list<std::shared_ptr<net>> m_nets;
        Settings m_settings;
        QMap<const connectable*, std::pair<wire*, int>> m_connections;
        std::function<std::shared_ptr<net>()> m_net_factory;

        [[nodiscard]]
        static
        bool
        merge_nets(std::shared_ptr<wire_system::net>& net, std::shared_ptr<wire_system::net>& otherNet);

        void
        detach_wire_from_all(const wire* wire);

        [[nodiscard]]
        std::shared_ptr<net>
        create_net();
    };

}
