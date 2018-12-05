#pragma once

#include <QUndoCommand>
#include <QPointer>
#include <memory>

namespace QSchematic
{
    class Node;
    class Connector;
}

class CommandNodeAddConnector : public QUndoCommand
{
public:
    CommandNodeAddConnector(const QPointer<QSchematic::Node>& node, std::unique_ptr<QSchematic::Connector> connector, QUndoCommand* parent = nullptr);

    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand* command) override;
    virtual void undo() override;
    virtual void redo() override;

private:
    QPointer<QSchematic::Node> _node;
    std::unique_ptr<QSchematic::Connector> _connector;
};
