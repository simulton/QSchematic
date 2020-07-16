#pragma once

#include "qschematic/commands/commandbase.h"
#include <QPointer>
#include <memory>

namespace QSchematic
{
    class Node;
    class Connector;
}

class CommandNodeAddConnector : public QSchematic::UndoCommand
{
public:
    CommandNodeAddConnector(const QPointer<QSchematic::Node>& node, const std::shared_ptr<QSchematic::Connector>& connector, QUndoCommand* parent = nullptr);

    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand* command) override;
    virtual void undo() override;
    virtual void redo() override;

private:
    QPointer<QSchematic::Node> _node;
    std::shared_ptr<QSchematic::Connector> _connector;
};
