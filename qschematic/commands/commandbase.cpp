#include "commands.h"
#include "commandbase.h"

using namespace QSchematic;

UndoCommand::UndoCommand(QUndoCommand* parent) : QUndoCommand(parent)
{
}

/**
 * @brief Pure convenience — reduce boilerplate clutter.
 */
void UndoCommand::connectDependencyDestroySignal(const QObject* dependency)
{
    connect(dependency, &QObject::destroyed, this, &UndoCommand::handleDependencyDestruction);
}

/**
 * @brief Sole purpose is to make it dead simple to tie a signal to obsoletion
 * while maintaining destroy tracking of `this`
 */
auto UndoCommand::handleDependencyDestruction(const QObject* dependency) -> void
{
    Q_UNUSED(dependency)
    setObsolete(true);
}
