#include "commands.hpp"
#include "scene_restore.hpp"
#include "../scene.hpp"

using namespace QSchematic;
using namespace QSchematic::Commands;

SceneRestore::SceneRestore(const QPointer<Scene>& scene, gpds::container before, const QString& text, QUndoCommand* parent) :
    Base(parent),
    _scene(scene),
    _before(std::move(before))
{
    connectDependencyDestroySignal(_scene.data());

    if (_scene)
        _after = _scene->to_container();

    setText(text.isEmpty() ? tr("Restore scene") : text);
}

int
SceneRestore::id() const
{
    return SceneRestoreCommandType;
}

void
SceneRestore::undo()
{
    if (!_scene)
        return;

    _scene->restore_from_container(_before);
}

void
SceneRestore::redo()
{
    // The scene already holds the redo state when the command is pushed
    if (!_pushed) {
        _pushed = true;
        return;
    }

    if (!_scene)
        return;

    _scene->restore_from_container(_after);
}
