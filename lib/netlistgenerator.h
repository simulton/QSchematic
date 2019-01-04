#pragma once

#include "netlist.h"
#include "scene.h"
#include "items/wirenet.h"
#include "items/node.h"

namespace QSchematic
{
    class Node;
    class Connector;
    class Scene;

    class NetlistGenerator
    {
    public:
        template<typename TNode = Node*, typename TConnector = Connector*>
        static Netlist<TNode, TConnector> generate(const Scene& scene)
        {
            struct GlobalNet
            {
                QString name;
                QList<std::shared_ptr<WireNet>> wireNets;
            };

            // Create a list of global nets (WireNets that share the same net name)
            QList<GlobalNet> globalNets;
            unsigned anonNetCounter = 0;
            for (const auto& wireNet : scene.nets()) {
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
                        newGlobalNet.name = QString("N%1").arg(anonNetCounter++, 3, 10, QChar('0'));
                    }

                    globalNets.append(newGlobalNet);
                }
            }

            // Export nets
            QVector<Net<TNode, TConnector>> nets;
            for (const auto& globalNet : globalNets) {
                // Create the new Net
                Net<TNode, TConnector> net;
                net.name = globalNet.name;

                // Build a list of all wire net points in scene coordinates
                QList<QPointF> wireScenePoints;
                for (const auto& wireNet : globalNet.wireNets) {
                    wireScenePoints << wireNet->points();
                }

                // Build a list of all connectors
                for (auto& node : scene.nodes()) {
                    // Convert to template node type
                    TNode templateNode = qgraphicsitem_cast<TNode>(node.get());
                    if (!templateNode) {
                        continue;
                    }

                    // Create list of all nodes in this net
                    net.nodes.push_back(templateNode);

                    // Loop through all Node's connectors
                    for (auto& connector : node->connectors()) {
                        // Convert to template connector type
                        TConnector templateConnector = qgraphicsitem_cast<TConnector>(connector.get());
                        if (!templateConnector) {
                            continue;
                        }

                        // Create a list of all connectors in this net
                        net.connectors.push_back(templateConnector);

                        // Create the Connector/Node pairs
                        auto connectorSceneConnectionPoint = templateConnector->connectionPoint();
                        if (wireScenePoints.contains(connectorSceneConnectionPoint)) {
                            net.connectorNodePairs.emplace(std::pair<TConnector, TNode>(templateConnector, templateNode));
                        }
                    }
                }

                nets << net;
            }

            return Netlist<TNode, TConnector>(nets);
        }

    private:
        NetlistGenerator() = default;
        NetlistGenerator(const NetlistGenerator& other) = default;
        NetlistGenerator(NetlistGenerator&& other) = default;
        virtual ~NetlistGenerator() = default;
    };

}
