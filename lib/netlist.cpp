#include <QJsonObject>
#include <QJsonArray>
#include "netlist.h"
#include "items/connector.h"
#include "items/node.h"
#include "items/label.h"

using namespace QSchematic;

Netlist::Netlist(const QVector<Net>& nets) :
    _nets(nets)
{
}

QJsonObject Netlist::toJson() const
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

QVector<Net> Netlist::nets() const
{
    return _nets;
}
