#pragma once

#include <vector>
#include <map>
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

    template<typename TNode, typename TConnector>
    struct Net
    {
        QString name;
        std::vector<TNode> nodes;
        std::vector<TConnector> connectors;
        std::map<TConnector, TNode> connectorNodePairs;
    };

    template<typename TNode, typename TConnector, typename TNet>
    class Netlist
    {
    public:
        Netlist(const QVector<TNet>& nets) :
            _nets(nets)
        {
        }

        Netlist( ) = default;
        Netlist(const Netlist& other) = default;
        Netlist(Netlist&& other) = default;
        virtual ~Netlist() = default;

        Netlist<TNode, TConnector, TNet>& operator=(const Netlist<TNode, TConnector, TNet>& rhs) = default;

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

        QVector<TNet> nets() const
        {
            return _nets;
        }

        QList<TNet> netsWithNode(const TNode node) const
        {
            // Sanity check
            if (!node) {
                return { };
            }

            // Loop
            QList<TNet> nets;
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

    private:
        QVector<TNet> _nets;
    };
}
