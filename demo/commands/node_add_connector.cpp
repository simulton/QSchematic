#include "commands.hpp"
#include "node_add_connector.hpp"

#include <qschematic/items/node.hpp>
#include <qschematic/items/connector.hpp>

using namespace Commands;

NodeAddConnector::NodeAddConnector(
    const QPointer<QSchematic::Items::Node>& node,
    std::shared_ptr<QSchematic::Items::Connector> connector,
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
