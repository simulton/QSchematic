#pragma once

#include <QUndoCommand>

namespace QSchematic
{
    class Scene;
    class Item;

    class UndoCommand :
        public QObject,
        public QUndoCommand
    {
    public:
        UndoCommand(QUndoCommand* parent = nullptr);
        virtual ~UndoCommand() = default;

        auto connectDependencyDestroySignal(const QObject* dependency) -> void;
        auto handleDependencyDestruction(const QObject* dependency) -> void;
    };

}
