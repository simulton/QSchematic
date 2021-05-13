#pragma once

#include "qschematic_export.h"

#include <QUndoCommand>

namespace QSchematic
{
    class Scene;
    class Item;

    class QSCHEMATIC_EXPORT UndoCommand :
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
