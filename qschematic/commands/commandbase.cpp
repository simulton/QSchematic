#include "commands.h"
#include "commandbase.h"
#include <QDebug>

using namespace QSchematic;

UndoCommand::UndoCommand(QUndoCommand* parent) :
      QUndoCommand(parent)
{
//    qDebug() << "UndoCommand::UndoCommand() ->" << this;
}

//UndoCommand::~UndoCommand()
//{
//    qDebug() << "UndoCommand::~UndoCommand() ->" << this;
//}

/**
 * @brief Pure convenience — reduce boilerplate clutter.
 */
void UndoCommand::connectDependencyDestroySignal(const QObject* dependency)
{
    qDebug() << endl << "UndoCommand::connectDependencyDestroySignal ->" << dependency << endl;
    connect(dependency, &QObject::destroyed, this, &UndoCommand::handleDependencyDestruction);
}

/**
 * @brief Sole purpose is to make it dead simple to tie a signal to obsoletion
 * while maintaining destroy tracking of `this`
 */
auto UndoCommand::handleDependencyDestruction(const QObject* dependency) -> void
{
    Q_UNUSED(dependency)
    qDebug() << endl << "UndoCommand::handleDependencyDestruction ->" << dependency << endl;
    setObsolete(true);
}
