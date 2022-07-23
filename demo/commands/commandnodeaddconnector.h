#pragma once

#include <qschematic/commands/commandbase.h>
#include <QPointer>

#include <memory>

namespace QSchematic
{
    class Node;
    class Connector;
}

namespace Commands
{

    class CommandNodeAddConnector :
        public QSchematic::UndoCommand
    {
    public:
        CommandNodeAddConnector(const QPointer<QSchematic::Node>& node, std::shared_ptr<QSchematic::Connector> connector, QUndoCommand* parent = nullptr);

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        QPointer<QSchematic::Node> _node;
        std::shared_ptr<QSchematic::Connector> _connector;
    };

}
