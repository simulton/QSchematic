#pragma once

#include <QUndoCommand>

namespace QSchematic
{
    class Scene;
    class Item;

    class UndoCommand : public QUndoCommand, public QObject
    {
    public:
        UndoCommand(QUndoCommand* parent = nullptr);

        auto connectDependencyDestroySignal(QObject* dependency) -> void;
        auto handleDependencyDestruction(const QObject* dependency) -> void;
    };

}
