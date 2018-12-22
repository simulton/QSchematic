#pragma once

#include <memory>
#include <QVector>
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
    struct ConnectorWithNode
    {
        ConnectorWithNode() :
            _connector(nullptr),
            _node(nullptr)
        {
        }

        ConnectorWithNode(TConnector connector, TNode node) :
            _connector(connector),
            _node(node)
        {
        }

        TConnector _connector;
        TNode _node;
    };

    template<typename TNode, typename TConnector>
    struct Net
    {
        QString name;
        QVector<ConnectorWithNode<TNode, TConnector>> connectorWithNodes;
        QVector<TNode> nodes;
        QVector<TConnector> connectors;

        bool isValid() const
        {
            return !connectorWithNodes.isEmpty();
        }
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

                netObject.insert("name", net.name);

                QJsonArray netConnectionsArray;
                for (const auto& connectorWithNode : net.connectorWithNodes) {
                    QJsonObject connection;
                    connection.insert("node text", connectorWithNode._node->label()->text());
                    connection.insert("connector text", connectorWithNode._connector->text());
                    netConnectionsArray.append(connection);
                }
                netObject.insert("connections", netConnectionsArray);

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
