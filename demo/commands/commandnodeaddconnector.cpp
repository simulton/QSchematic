#include "commands.h"
#include "commandnodeaddconnector.h"

#include <qschematic/items/node.h>
#include <qschematic/items/connector.h>

using namespace Commands;

NodeAddConnector::NodeAddConnector(
    const QPointer<QSchematic::Node>& node,
    std::shared_ptr<QSchematic::Connector> connector,
    QUndoCommand* parent
) :
    QSchematic::Commands::Base(parent),
    _node(node),
    _connector(std::move(connector))
{
    connect(_node.data(), &QObject::destroyed, this, &QSchematic::Commands::Base::handleDependencyDestruction);
    setText(QStringLiteral("Add connector"));
}

int
NodeAddConnector::id() const
{
    return NodeAddConnectorCommandType;
}

bool
NodeAddConnector::mergeWith(const QUndoCommand* command)
{
    Q_UNUSED(command)

    return false;
}

void
NodeAddConnector::undo()
{
    if (!_node || !_connector)
        return;

    _node->removeConnector(_connector);
    _connector->setVisible(false);
}

void
NodeAddConnector::redo()
{
    if (!_node || !_connector)
        return;

    _node->addConnector(_connector);
    _connector->setVisible(true);
}
