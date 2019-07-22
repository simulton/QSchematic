#pragma once

#include <vector>
#include <forward_list>
#include <map>
#include <optional>
#include <QJsonObject>
#include <QJsonArray>
#include "items/wire.h"
#include "items/connector.h"
#include "items/node.h"
#include "items/label.h"

class QJsonObject;

namespace QSchematic
{
    class Node;
    class Connector;

    template<typename TWire, typename TNode, typename TConnector>
    struct Net
    {
        QString name;
        std::vector<TWire> wires;
        std::vector<TNode> nodes;
        std::vector<TConnector> connectors;
        std::map<TConnector, TNode> connectorNodePairs;
    };

    template<typename TNode, typename TConnector, typename TWire, typename TNet>
    class Netlist
    {
    public:
        Netlist( ) = default;
        Netlist(const Netlist& other) = default;
        Netlist(Netlist&& other) = default;
        virtual ~Netlist() = default;

        Netlist<TNode, TConnector, TWire, TNet>& operator=(const Netlist<TNode, TConnector, TWire, TNet>& rhs) = default;

        QJsonObject toJson() const
        {
            QJsonObject object;

            // Nets
            QJsonArray netsArray;
            for (const auto& net : _nets) {
                QJsonObject netObject;

                // Net name
                netObject.insert("name", net.name);

                // Connectors
                QJsonArray connectorsArray;
                for (const auto& connector : net.connectors) {
                    connectorsArray.append(connector->label()->text());
                }
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

        void set( std::vector<TNode>&& nodes, std::vector<TNet>&& nets )
        {
            _nodes = std::move( nodes );
            _nets = std::move( nets );
        }

        std::vector<TNet> nets() const
        {
            return _nets;
        }

        std::forward_list<TNet> netsWithNode(const TNode node) const
        {
            // Sanity check
            if (!node) {
                return { };
            }

            // Loop
            std::forward_list<TNet> nets;
            for (auto& net : _nets) {
                for (auto& connectorWithNode : net.connectorWithNodes) {
                    if (connectorWithNode._node == node) {
                        nets << net;
                        break;
                    }
                }
            }

            return nets;
        }

        std::optional<TNet> netFromConnector(const TConnector connector) const
        {
            // Sanity check
            if (not connector) {
                return std::nullopt;
            }

            // Loop
            for (auto& net : _nets) {
                for (auto& c : net.connectors) {
                    if (c == connector) {
                        return net;
                    }
                }
            }

            return std::nullopt;
        }

        std::vector<TNode> nodes() const
        {
            return _nodes;
        }

    private:
        std::vector<TNode> _nodes;
        std::vector<TNet> _nets;
    };
}
