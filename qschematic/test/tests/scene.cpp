#include <doctest.h>

#include "../helpers.hpp"
#include "../../commands/scene_restore.hpp"
#include "../../scene.hpp"

#include <QUndoStack>

using namespace QSchematic;

TEST_SUITE("Scene")
{
    TEST_CASE("from_container(): Adds to the existing content")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto container = scene.to_container();
        scene.from_container(container);

        CHECK_EQ(scene.items().size(), 2);
    }

    TEST_CASE("from_container(): Clears the undo stack")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto container = scene.to_container();
        scene.undoStack()->push(new Commands::SceneRestore(&scene, container));
        REQUIRE_EQ(scene.undoStack()->count(), 1);

        scene.from_container(container);

        CHECK_EQ(scene.undoStack()->count(), 0);
    }

    TEST_CASE("restore_from_container(): Replaces the existing content")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto container = scene.to_container();
        scene.addItem(test::make_node(100, 0));
        REQUIRE_EQ(scene.items().size(), 2);

        scene.restore_from_container(container);

        CHECK_EQ(scene.items().size(), 1);
    }

    TEST_CASE("restore_from_container(): Leaves the undo stack alone")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto container = scene.to_container();
        scene.undoStack()->push(new Commands::SceneRestore(&scene, container));
        REQUIRE_EQ(scene.undoStack()->count(), 1);

        scene.restore_from_container(container);

        CHECK_EQ(scene.undoStack()->count(), 1);
    }

    TEST_CASE("restore_from_container(): Leaves the clean state alone")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));

        const auto container = scene.to_container();
        scene.undoStack()->push(new Commands::SceneRestore(&scene, container));
        REQUIRE_FALSE(scene.undoStack()->isClean());

        scene.restore_from_container(container);

        CHECK_FALSE(scene.undoStack()->isClean());
    }

    TEST_CASE("clear(): Clears the undo stack")
    {
        Scene scene;
        scene.addItem(test::make_node(0, 0));
        scene.undoStack()->push(new Commands::SceneRestore(&scene, scene.to_container()));
        REQUIRE_EQ(scene.undoStack()->count(), 1);

        scene.clear();

        CHECK_EQ(scene.undoStack()->count(), 0);
        CHECK(scene.undoStack()->isClean());
        CHECK_EQ(scene.items().size(), 0);
    }
}
