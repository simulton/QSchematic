#pragma once

#include <memory>
#include <QVector>

class QJsonObject;

namespace QSchematic
{
    class Node;
    class Connector;

    struct ConnectorWithNode
    {
        std::shared_ptr<const Connector> _connector;
        std::shared_ptr<const Node> _node;
    };

    class Netlist
    {
    public:
        Netlist(const QVector<ConnectorWithNode>& nets);
        Netlist(const Netlist& other) = default;
        Netlist(Netlist&& other) = default;
        virtual ~Netlist() = default;

        QJsonObject toJson() const;

        QVector<ConnectorWithNode> nets() const;

    private:
        QVector<ConnectorWithNode> _nets;
    };

}
