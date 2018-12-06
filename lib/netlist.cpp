#include <QJsonObject>
#include "netlist.h"

using namespace QSchematic;

Netlist::Netlist(const QVector<ConnectorWithNode>& nets) :
    _nets(nets)
{
}

QJsonObject Netlist::toJson() const
{
    QJsonObject object;

    return object;
}

QVector<ConnectorWithNode> Netlist::nets() const
{
    return _nets;
}
