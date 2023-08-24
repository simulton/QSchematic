#pragma once

#include <QUndoCommand>

namespace QSchematic::Commands
{
    class Base :
        public QObject,
        public QUndoCommand
    {
    public:
        explicit
        Base(QUndoCommand* parent = nullptr);

        virtual
        ~Base() = default;

        /**
         * @brief Pure convenience — reduce boilerplate clutter.
         */
        void
        connectDependencyDestroySignal(const QObject* dependency);

        /**
         * @brief Sole purpose is to make it dead simple to tie a signal to obsoletion
         * while maintaining destroy tracking of `this`
         */
        void
        handleDependencyDestruction(const QObject* dependency);
    };

}
