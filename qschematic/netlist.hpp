#pragma once

#include "items/wire.hpp"
#include "items/connector.hpp"
#include "items/node.hpp"
#include "items/label.hpp"

#include <vector>
#include <forward_list>
#include <map>
#include <optional>

namespace QSchematic
{

    template<
        typename TWire = Items::Wire*,
        typename TNode = Items::Node*,
        typename TConnector = Items::Connector*
    >
    struct Net
    {
        QString name;
        std::vector<TWire> wires;
        std::vector<TNode> nodes;
        std::vector<TConnector> connectors;
        std::map<TConnector, TNode> connectorNodePairs;
    };

    template<
        typename TNode = Items::Node*,
        typename TConnector = Items::Connector*,
        typename TWire = Items::Wire*,
        typename TNet = Net<TWire, TNode, TConnector>
    >
    class Netlist
    {
    public:
        std::vector<TNode> nodes;
        std::vector<TNet> nets;

        Netlist() = default;
        Netlist(const Netlist& other) = default;
        Netlist(Netlist&& other) = default;
        virtual ~Netlist() = default;

        Netlist<TNode, TConnector, TWire, TNet>&
        operator=(const Netlist<TNode, TConnector, TWire, TNet>& rhs) = default;

        std::forward_list<TNet>
        netsWithNode(const TNode node) const
        {
            // Sanity check
            if (!node)
                return { };

            // Loop
            std::forward_list<TNet> nets;
            for (auto& net : nets) {
                for (auto& connectorWithNode : net.connectorWithNodes) {
                    if (connectorWithNode._node == node) {
                        nets << net;
                        break;
                    }
                }
            }

            return nets;
        }

        std::optional<TNet>
        netFromConnector(const TConnector connector) const
        {
            // Sanity check
            if (connector)
                return std::nullopt;

            // Loop
            for (auto& net : nets) {
                for (auto& c : net.connectors) {
                    if (c == connector)
                        return net;
                }
            }

            return std::nullopt;
        }
    };
}
