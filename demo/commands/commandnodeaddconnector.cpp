#include <qschematic/items/node.h>
#include <qschematic/items/connector.h>
#include "commands.h"
#include "commandnodeaddconnector.h"

CommandNodeAddConnector::CommandNodeAddConnector(const QPointer<QSchematic::Node>& node, const std::shared_ptr<QSchematic::Connector>& connector, QUndoCommand* parent) :
    UndoCommand(parent),
    _node(node),
    _connector(connector)
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
    if (!_node or !_connector) {
        return;
    }

    _node->removeConnector(_connector);
    _connector->setVisible(false);
}

void CommandNodeAddConnector::redo()
{
    if (!_node or !_connector) {
        return;
    }

    _node->addConnector(_connector);
    _connector->setVisible(true);
}
