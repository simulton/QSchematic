#pragma once

#include "base.hpp"

#include <gpds/container.hpp>
#include <QPointer>
#include <QString>

namespace QSchematic
{
    class Scene;
}

namespace QSchematic::Commands
{
    /**
     * Replace the entire scene content as one undo step.
     *
     * @details This covers bulk operations that are impractical to express as a series of item
     *          commands, such as importing a document. Capture the scene, perform the operation,
     *          then push the command:
     *
     *          @code
     *          const auto before = scene->to_container();
     *          import_document(scene);
     *          scene->undoStack()->push(new Commands::SceneRestore(scene, before, tr("Import")));
     *          @endcode
     *
     * @note The constructor captures the current scene as the redo state, so the redo() that
     *       QUndoStack::push() performs does nothing.
     */
    class SceneRestore :
        public Base
    {
    public:
        SceneRestore(const QPointer<Scene>& scene, gpds::container before, const QString& text = {}, QUndoCommand* parent = nullptr);

        int id() const override;
        void undo() override;
        void redo() override;

    private:
        QPointer<Scene> _scene;
        gpds::container _before;
        gpds::container _after;
        bool _pushed = false;
    };

}
