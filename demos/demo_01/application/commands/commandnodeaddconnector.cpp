#include "../../../lib/items/node.h"
#include "../../../lib/items/connector.h"
#include "commands.h"
#include "commandnodeaddconnector.h"

CommandNodeAddConnector::CommandNodeAddConnector(const QPointer<QSchematic::Node>& node, std::unique_ptr<QSchematic::Connector> connector, QUndoCommand* parent) :
    QUndoCommand(parent),
    _node(node),
    _connector(std::move(connector))
{
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

#warning ToDo
    //_node->removeConnector(_connector->connectionPoint());
}

void CommandNodeAddConnector::redo()
{
    if (!_node or !_connector) {
        return;
    }

    auto connectorClone = qgraphicsitem_cast<QSchematic::Connector*>(_connector->deepCopy().release());
    _node->addConnector(std::unique_ptr<QSchematic::Connector>(connectorClone));
}
