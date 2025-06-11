#pragma once

#include "netlist.hpp"
#include "scene.hpp"
#include "items/wirenet.hpp"
#include "items/node.hpp"

namespace QSchematic
{
    class Wire;
    class Node;
    class Connector;
    class Scene;

    class NetlistGenerator
    {
    public:
        template<
            typename TNode = Node*,
            typename TConnector = Connector*,
            typename TWire = Wire*,
            typename TNet = Net<TWire, TNode, TConnector>
        >
        static
        bool
        generate(Netlist<TNode, TConnector, TWire, TNet>& netlist, const Scene& scene)
        {
            struct GlobalNet
            {
                QString name;
                QList<std::shared_ptr<Items::WireNet>> wireNets;
            };

            // Get the wiresystem manager
            auto wm = scene.wire_manager();
            if (!wm)
                return false;

            // Add all nodes
            std::vector<TNode> nodes;
            for (const auto& node : scene.nodes()) {
                // Sanity check
                if (!node)
                    continue;

                nodes.push_back( static_cast<TNode>( node.get() ) );
            }

            // Create a list of global nets (WireNets that share the same net name)
            std::vector<GlobalNet> globalNets;
            unsigned anonNetCounter = 0;
            for (const auto& net : wm->nets()) {

                // Get the wire net
                auto wireNet = std::dynamic_pointer_cast<Items::WireNet>(net);
                if (!wireNet)
                    continue;

                // Figure out whether we need a new global net
                bool createNewNet = true;
                for (auto& globalNet : globalNets) {
                    if (wireNet->name().isEmpty()) {
                        createNewNet = true;
                        break;
                    }
                    if (QString::compare(globalNet.name, wireNet->name(), Qt::CaseSensitive) == 0) {
                        globalNet.wireNets.append(wireNet);
                        createNewNet = false;
                        break;
                    }
                }

                // Create a new net if supposed to
                if (createNewNet) {
                    GlobalNet newGlobalNet;
                    newGlobalNet.wireNets.append(wireNet);
                    newGlobalNet.name = wireNet->name();

                    // Prevent empty names
                    if (newGlobalNet.name.isEmpty())
                        newGlobalNet.name = QString("N%1").arg(anonNetCounter++, 3, 10, QChar('0'));

                    globalNets.push_back(newGlobalNet);
                }
            }

            // Export nets
            std::vector<TNet> nets;
            for (const auto& globalNet : globalNets) {
                // Create the new Net
                TNet net;
                net.name = globalNet.name;

                // Build a list of all wire net points in scene coordinates
                QList<QPointF> wireScenePoints;
                for (const auto& wireNet : globalNet.wireNets) {
                    wireScenePoints << wireNet->points();

                    // Store wires
                    for ( const auto& wire : wireNet->wires()) {
                        TWire w = qobject_cast<TWire>( std::dynamic_pointer_cast<Items::Wire>(wire).get() );
                        if (w)
                            net.wires.push_back( w );
                    }
                }

                // Build a list of all connectors
                for (auto& node : scene.nodes()) {
                    // Convert to template node type
                    TNode templateNode = qgraphicsitem_cast<TNode>(node.get());
                    if (!templateNode)
                        continue;

                    // Loop through all Node's connectors
                    for (auto& connector : node->connectors()) {
                        // Convert to template connector type
                        TConnector templateConnector = qgraphicsitem_cast<TConnector>(connector.get());
                        if (!templateConnector)
                            continue;

                        // Create the Connector/Node pairs
                        const auto* wire = wm->attached_wire(connector.get());
                        if (std::find(net.wires.begin(), net.wires.end(), wire) != net.wires.end()) {

                            // Create list of all nodes in this net
                            net.nodes.push_back(templateNode);

                            // Create a list of all connectors in this net
                            net.connectors.push_back(templateConnector);

                            // Connector/Node pairs
                            net.connectorNodePairs.emplace(std::pair<TConnector, TNode>(templateConnector, templateNode));
                        }
                    }
                }

                // Check if the net makes a connection
                // A net is considered to make a connection if at least two wire points are on connectors.
                // Note: This implicitly also ensures that the connectors are individual/separate connectors as long as we
                //       ensure that the net::connectors collection does not contain duplicate items.
                bool netMakesConnection = false;
                int connectionsCount = 0;
                for (const auto& conn : net.connectors) {
                    if (conn->hasConnection())
                        connectionsCount++;
                }
                netMakesConnection = connectionsCount >= 2;

                // Add the net (if supposed to)
                if (netMakesConnection)
                    nets.push_back( net );
            }

            // Set the netlist
            netlist.nodes = std::move(nodes);
            netlist.nets = std::move(nets);

            return true;
        }

    private:
        NetlistGenerator() = default;
        NetlistGenerator(const NetlistGenerator& other) = default;
        NetlistGenerator(NetlistGenerator&& other) = default;
        virtual ~NetlistGenerator() = default;
    };

}
