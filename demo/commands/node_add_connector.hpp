#pragma once

#include <qschematic/commands/base.hpp>
#include <QPointer>

#include <memory>

namespace QSchematic::Items
{
    class Node;
    class Connector;
}

namespace Commands
{

    class NodeAddConnector :
        public QSchematic::Commands::Base
    {
    public:
        NodeAddConnector(
            const QPointer<QSchematic::Items::Node>& node,
            std::shared_ptr<QSchematic::Items::Connector> connector,
            QUndoCommand* parent = nullptr
        );

        int id() const override;
        bool mergeWith(const QUndoCommand* command) override;
        void undo() override;
        void redo() override;

    private:
        QPointer<QSchematic::Items::Node> _node;
        std::shared_ptr<QSchematic::Items::Connector> _connector;
    };

}
