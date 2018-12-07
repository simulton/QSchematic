#include "netlistgenerator.h"
#include "netlist.h"
#include "scene.h"
#include "items/wirenet.h"
#include "items/node.h"

using namespace QSchematic;

Netlist NetlistGenerator::generate(const Scene& scene)
{
    struct GlobalNet
    {
        QString name;
        QList<const WireNet*> wireNets;
    };

    // Create a list of global nets (WireNets that share the same net name)
    QList<GlobalNet> globalNets;
    for (const WireNet* wireNet : scene.nets()) {
        // Sanity check
        if (!wireNet) {
            continue;
        }

        // Figure out whether we need a new global net
        bool createNewNet = true;
        for (auto& globalNet : globalNets) {
            if (wireNet->name().isEmpty()) {
                createNewNet = true;
                break;
            }
            if (QString::compare(globalNet.name, wireNet->name()) == 0) {
                globalNet.wireNets.append(wireNet);
                createNewNet = false;
                break;
            }
        }

        // Create a new net if supposed to
        if (createNewNet) {
            GlobalNet newGlobalNet;
            newGlobalNet.wireNets.append(wireNet);
            newGlobalNet.name = wireNet->name();

            // Prevent empty names
            if (newGlobalNet.name.isEmpty()) {
                newGlobalNet.name = QString("N%1").arg(globalNets.count()+1, 3, 10, QChar('0'));
            }

            globalNets.append(newGlobalNet);
        }
    }

    // Export nets
    QVector<Net> nets;
    for (const auto& globalNet : globalNets) {
        Net net;
        net.name = globalNet.name;

        // Build a list of all wire net points in scene coordinates
        QList<QPoint> wireScenePoints;
        for (const auto& wireNet : globalNet.wireNets) {
            wireScenePoints << wireNet->points();
        }

        // Build a list of all connectors
        for (const auto& node : scene.nodes()) {
            for (const auto& connector : node->connectors()) {
                auto connectorSceneConnectionPoint = connector->connectionPoint();
                if (wireScenePoints.contains(connectorSceneConnectionPoint)) {
                    net.connectorWithNodes << ConnectorWithNode(connector.get(), node);
                }
            }
        }

        nets << net;
    }

    return Netlist(nets);
}
