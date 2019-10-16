#pragma once

#include <QUndoCommand>

namespace QSchematic
{
    class Scene;
    class Item;

    // NOTE: it would be prudent to keep QObject away from UndoCommand to keep
    // it neat and tight for long undo-history, but this is the fastest way to
    // make the code-base stable, so let's go for it for now, optimize if needed
    class UndoCommand : public QUndoCommand, public QObject
    {
    public:
        UndoCommand(QUndoCommand* parent = nullptr);

        auto connectDependencyDestroySignal(QObject* dependency) -> void;
        auto handleDependencyDestruction(const QObject* dependency) -> void;
    };

}
