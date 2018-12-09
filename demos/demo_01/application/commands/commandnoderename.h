#pragma once

#include <QUndoCommand>
#include <QPointer>
#include <memory>

namespace QSchematic
{
    class Node;
    class Connector;
}

class CommandNodeRename : public QUndoCommand
{
public:
    CommandNodeRename(const QPointer<QSchematic::Node>& node, const QString& newText, QUndoCommand* parent = nullptr);

    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand* command) override;
    virtual void undo() override;
    virtual void redo() override;

private:
    QPointer<QSchematic::Node> _node;
    QString _oldText;
    QString _newText;
};
