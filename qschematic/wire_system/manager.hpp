#pragma once

#include "../settings.hpp"

#include <QObject>

#include <list>
#include <memory>
#include <optional>
#include <unordered_map>
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
        /**
         * Structure used to record a connection of a wire.
         */
        struct connection_record
        {
            /// The wire
            class wire* wire = nullptr;

            /// The index of the point of the wire
            int point_index = -1;
        };

        /**
         * Structure to represent zero or more nets which share the same name.
         */
        struct global_net
        {
            std::string name;
            std::vector<std::shared_ptr<net>> nets;
        };

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

        /**
         * Return a collection of all nets.
         */
        [[nodiscard]]
        std::vector<std::shared_ptr<net>>
        nets() const;

        /**
         * Return a collection of all global nets.
         *
         * Anonymous (unnamed nets) will be assigned an auto-generated global net name.
         */
        [[nodiscard]]
        std::vector<global_net>
        global_nets() const;

        [[nodiscard]]
        std::vector<std::shared_ptr<wire>>
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
        std::list<std::shared_ptr<wire>>
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
         * Checks if a specified wire is connected to a specified connector.
         */
        [[nodiscard]]
        bool
        is_wire_attached_to(const wire* wire, const connectable* connector);

        /**
         * Get the connection record for a specified connector (if any).
         */
        [[nodiscard]]
        std::optional<connection_record>
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
        std::vector<std::shared_ptr<net>> m_nets;
        Settings m_settings;
        std::function<std::shared_ptr<net>()> m_net_factory;
        std::unordered_map<const connectable*, connection_record> m_connections;

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
