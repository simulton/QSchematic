#include "commands.h"
#include "commandnodeaddconnector.h"

#include <qschematic/items/node.h>
#include <qschematic/items/connector.h>

using namespace Commands;

CommandNodeAddConnector::CommandNodeAddConnector(const QPointer<QSchematic::Node>& node, std::shared_ptr<QSchematic::Connector> connector, QUndoCommand* parent) :
    UndoCommand(parent),
    _node(node),
    _connector(std::move(connector))
{
    connect(_node.data(), &QObject::destroyed, this, &UndoCommand::handleDependencyDestruction);
    setText(QStringLiteral("Add connector"));
}

int CommandNodeAddConnector::id() const
{
    return NodeAddConnectorCommandType;
}

bool CommandNodeAddConnector::mergeWith(const QUndoCommand* command)
{
    Q_UNUSED(command)

    return false;
}

void CommandNodeAddConnector::undo()
{
    if (!_node || !_connector)
        return;

    _node->removeConnector(_connector);
    _connector->setVisible(false);
}

void CommandNodeAddConnector::redo()
{
    if (!_node || !_connector)
        return;

    _node->addConnector(_connector);
    _connector->setVisible(true);
}
