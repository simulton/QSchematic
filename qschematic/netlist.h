#pragma once

#include "items/wire.h"
#include "items/connector.h"
#include "items/node.h"
#include "items/label.h"

#include <QJsonObject>
#include <QJsonArray>

#include <vector>
#include <forward_list>
#include <map>
#include <optional>

class QJsonObject;

namespace QSchematic
{
    class Node;
    class Connector;

    template<
        typename TWire = Wire*,
        typename TNode = Node*,
        typename TConnector = Connector*
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
        typename TNode = Node*,
        typename TConnector = Connector*,
        typename TWire = Wire*,
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

        Netlist<TNode, TConnector, TWire, TNet>& operator=(const Netlist<TNode, TConnector, TWire, TNet>& rhs) = default;

        QJsonObject
        toJson() const
        {
            QJsonObject object;

            // Nets
            QJsonArray netsArray;
            for (const auto& net : nets) {
                QJsonObject netObject;

                // Net name
                netObject.insert("name", net.name);

                // Connectors
                QJsonArray connectorsArray;
                for (const auto& connector : net.connectors)
                    connectorsArray.append(connector->label()->text());
                netObject.insert("connectors", connectorsArray);

                // ConnectorNodePairs
                QJsonArray netConnectionsArray;
                for (auto it = net.connectorNodePairs.cbegin(); it != net.connectorNodePairs.cend(); it++) {
                    QJsonObject connection;
                    connection.insert("connector text", it->first->text());
                    netConnectionsArray.append(connection);
                }
                netObject.insert("connector node pairs", netConnectionsArray);

                netsArray.append(netObject);
            }
            object.insert("nets", netsArray);

            return object;
        }

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
