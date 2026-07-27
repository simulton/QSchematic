#include <doctest.h>

#include "../helpers.hpp"
#include "../../commands/scene_restore.hpp"
#include "../../scene.hpp"

#include <QUndoStack>

using namespace QSchematic;

TEST_SUITE("Commands::SceneRestore")
{
    TEST_CASE("Pushing the command leaves the scene as it is")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto before = scene.to_container();
        const auto node = test::make_node(100, 0);
        scene.addItem(node);

        scene.undoStack()->push(new Commands::SceneRestore(&scene, before));

        CHECK_EQ(scene.items().size(), 2);
        CHECK_EQ(scene.undoStack()->count(), 1);

        // Pushing must not rebuild the items and invalidate what the caller holds
        CHECK(scene.items().contains(node));
    }

    TEST_CASE("A bulk change is undone in one step")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto before = scene.to_container();
        for (int i = 0; i < 20; i++)
            scene.addItem(test::make_node(i * 10, 100));
        REQUIRE_EQ(scene.items().size(), 21);

        scene.undoStack()->push(new Commands::SceneRestore(&scene, before));
        scene.undo();

        CHECK_EQ(scene.items().size(), 1);
    }

    TEST_CASE("The command survives its own undo")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto before = scene.to_container();
        scene.addItem(test::make_node(100, 0));

        scene.undoStack()->push(new Commands::SceneRestore(&scene, before));
        scene.undo();

        CHECK_EQ(scene.undoStack()->count(), 1);
        CHECK(scene.undoStack()->isClean());
    }

    TEST_CASE("Undo and redo can be repeated")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto before = scene.to_container();
        scene.addItem(test::make_node(100, 0));
        scene.undoStack()->push(new Commands::SceneRestore(&scene, before));

        for (int i = 0; i < 3; i++) {
            scene.undo();
            CHECK_EQ(scene.items().size(), 1);

            scene.redo();
            CHECK_EQ(scene.items().size(), 2);
        }
    }

    TEST_CASE("The text is used as the undo step description")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        scene.undoStack()->push(new Commands::SceneRestore(&scene, scene.to_container(), "Import"));

        CHECK_EQ(scene.undoStack()->text(0), QString("Import"));
    }
}
