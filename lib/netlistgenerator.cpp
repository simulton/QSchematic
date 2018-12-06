#include "netlistgenerator.h"
#include "netlist.h"
#include "scene.h"

using namespace QSchematic;

Netlist NetlistGenerator::generate(const Scene& scene)
{
    QVector<ConnectorWithNode> nets;

    return Netlist(nets);
}
