#pragma once

#include "netlist.h"

#include <QJsonArray>
#include <QJsonObject>

namespace QSchematic
{

    template<typename Netlist>
    [[nodiscard]]
    QJsonObject
    toJson(const Netlist& nl)
    {
        QJsonObject object;

        // Nodes
        QJsonArray nodes;

        // Nets
        QJsonArray netsArray;
        for (const auto& net : nl.nets) {
            QJsonObject netObject;

            // Net name
            netObject.insert(QStringLiteral("name"), net.name);

            // Connections
            QJsonArray connectionsArray;
            for (const auto& [connector, node] : net.connectorNodePairs) {
                QJsonObject connection;
                connection.insert(QStringLiteral("node"), node->text());
                connection.insert(QStringLiteral("connector"), connector->label()->text());  // ToDo: Nullptr check
                connectionsArray.append(connection);
            }
            netObject.insert(QStringLiteral("connections"), connectionsArray);

            netsArray.append(netObject);
        }
        object.insert(QStringLiteral("nets"), netsArray);

        return object;
    }

}
