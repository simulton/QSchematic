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

    template<typename TNode, typename TConnector>
    class Netlist
    {
    public:
        Netlist(const QVector<Net<TNode, TConnector>>& nets) :
            _nets(nets)
        {
        }

        Netlist(const Netlist& other) = default;
        Netlist(Netlist&& other) = default;
        virtual ~Netlist() = default;

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

        QVector<Net<TNode, TConnector>> nets() const
        {
            return _nets;
        }

        QList<Net<TNode, TConnector>> netsWithNode(const TNode node) const
        {
            // Sanity check
            if (!node) {
                return { };
            }

            // Loop
            QList<Net<TNode, TConnector>> nets;
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
        QVector<Net<TNode, TConnector>> _nets;
    };
}
