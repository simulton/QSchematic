#pragma once

#include <memory>
#include <QVector>
#include "items/wire.h"

class QJsonObject;

namespace QSchematic
{
    class Node;
    class Connector;

    struct ConnectorWithNode
    {
        ConnectorWithNode() :
            _connector(nullptr),
            _node(nullptr)
        {
        }

        ConnectorWithNode(const Connector* const connector, const Node* const node) :
            _connector(connector),
            _node(node)
        {
        }

        const Connector* const _connector;
        const Node* const _node;
    };

    struct Net
    {
        QString name;
        QVector<ConnectorWithNode> connectorWithNodes;
    };

    class Netlist
    {
    public:
        Netlist(const QVector<Net>& nets);
        Netlist(const Netlist& other) = default;
        Netlist(Netlist&& other) = default;
        virtual ~Netlist() = default;

        QJsonObject toJson() const;

        QVector<Net> nets() const;

    private:
        QVector<Net> _nets;
    };

}
