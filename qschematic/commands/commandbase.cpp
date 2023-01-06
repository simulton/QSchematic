#include "commands.h"
#include "commandbase.h"

using namespace QSchematic::Commands;

Base::Base(QUndoCommand* parent) :
    QUndoCommand(parent)
{
}

void
Base::connectDependencyDestroySignal(const QObject* dependency)
{
    connect(dependency, &QObject::destroyed, this, &Base::handleDependencyDestruction);
}

void
Base::handleDependencyDestruction(const QObject* dependency)
{
    Q_UNUSED(dependency)

    setObsolete(true);
}
