#include "../3rdparty/doctest.h"
#include "../../manager.h"
#include "../../wire.h"

#include <QVector2D>

TEST_SUITE("Wire")
{
    wire_system::manager manager;

    TEST_CASE("Add and remove points")
    {
        // Create a wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point(QPointF(0, 10));
        wire->append_point(QPointF(23.2, 100));
        wire->append_point(QPointF(-123.4, 0.23));

        SUBCASE("Point can be added")
        {
            // Make sure the points are correct
            REQUIRE(wire->points_count() == 3);
            REQUIRE(wire->points().at(0) == QPointF(0, 10));
            REQUIRE(wire->points().at(1) == QPointF(23.2, 100));
            REQUIRE(wire->points().at(2) == QPointF(-123.4, 0.23));
        }

        SUBCASE("Points can be removed")
        {
            wire->remove_point(1);

            // Make sure the points has been removed
            REQUIRE(wire->points_count() == 2);
            REQUIRE(wire->points().at(0) == QPointF(0, 10));
            REQUIRE(wire->points().at(1) == QPointF(-123.4, 0.23));
        }
    }

    TEST_CASE("Wire can be simplified")
    {
        // Create a wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point(QPointF(10, 10));
        wire->append_point(QPointF(23.2, 10));
        wire->append_point(QPointF(1000, 10));
        wire->append_point(QPointF(1000, 10));

        REQUIRE(wire->points_count() == 4);

        wire->simplify();

        REQUIRE(wire->points_count() == 2);
    }

    TEST_CASE("Wires can be moved")
    {
        // Use a grid size of 1
        Settings settings = manager.settings();
        settings.gridSize = 1;
        manager.set_settings(settings);

        // Create a wire
        auto wire1 = std::make_shared<wire_system::wire>();
        wire1->append_point(QPointF(0, 10));
        wire1->append_point(QPointF(5, 10));
        wire1->append_point(QPointF(5, 5));
        wire1->append_point(QPointF(15, 5));
        manager.add_wire(wire1);

        // Create a second wire
        auto wire2 = std::make_shared<wire_system::wire>();
        wire2->append_point(QPointF(10, 5));
        wire2->append_point(QPointF(10, 10));
        wire2->append_point(QPointF(15, 10));
        manager.add_wire(wire2);

        // Generate the junctions
        manager.generate_junctions();

        // Move the first wire
        wire1->move(QVector2D(1, -1));

        // Make sure the first wire has moved correctly
        REQUIRE(wire1->points().at(0).toPointF() == QPointF(1, 9));
        REQUIRE(wire1->points().at(1).toPointF() == QPointF(6, 9));
        REQUIRE(wire1->points().at(2).toPointF() == QPointF(6, 4));
        REQUIRE(wire1->points().at(3).toPointF() == QPointF(16, 4));

        // Make sure the second wire stayed on the first one
        REQUIRE(wire2->points().at(0).toPointF() == QPointF(11, 4));
        REQUIRE(wire2->points().at(1).toPointF() == QPointF(11, 10));
        REQUIRE(wire2->points().at(2).toPointF() == QPointF(15, 10));

        // Move the second wire
        wire2->move(QVector2D(2, 1));

        // Make sure the first wire hasn't changed
        REQUIRE(wire1->points().at(0).toPointF() == QPointF(1, 9));
        REQUIRE(wire1->points().at(1).toPointF() == QPointF(6, 9));
        REQUIRE(wire1->points().at(2).toPointF() == QPointF(6, 4));
        REQUIRE(wire1->points().at(3).toPointF() == QPointF(16, 4));

        // Make sure the second wire has moved and is still on the first one
        REQUIRE(wire2->points().at(0).toPointF() == QPointF(11, 4));
        REQUIRE(wire2->points().at(1).toPointF() == QPointF(11, 11));
        REQUIRE(wire2->points().at(2).toPointF() == QPointF(17, 11));
    }

    TEST_CASE("point_is_on_wire()")
    {
        // Create a wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point(QPointF(0, 20));
        wire->append_point(QPointF(10, 20));
        wire->append_point(QPointF(10, 0));
        wire->append_point(QPointF(5, 10));
        wire->append_point(QPointF(20, 10));
        manager.add_wire(wire);

        // These points are on the wire
        REQUIRE(wire->point_is_on_wire(QPointF(10, 20)));
        REQUIRE(wire->point_is_on_wire(QPointF(10, 10)));
        REQUIRE(wire->point_is_on_wire(QPointF(15, 10)));
        REQUIRE(wire->point_is_on_wire(QPointF(12.34, 10.01)));
        REQUIRE(wire->point_is_on_wire(QPointF(20.01, 10)));
        REQUIRE(wire->point_is_on_wire(QPointF(6, 8)));
        REQUIRE(wire->point_is_on_wire(QPointF(8.2345, 3.531)));

        // These points are not on the wire
        REQUIRE_FALSE(wire->point_is_on_wire(QPointF(-10, -100)));
        REQUIRE_FALSE(wire->point_is_on_wire(QPointF(5, 5)));
        REQUIRE_FALSE(wire->point_is_on_wire(QPointF(6, 11)));
        REQUIRE_FALSE(wire->point_is_on_wire(QPointF(15, 15)));
        REQUIRE_FALSE(wire->point_is_on_wire(QPointF(15, 11)));
        REQUIRE_FALSE(wire->point_is_on_wire(QPointF(15, 10.1)));
        REQUIRE_FALSE(wire->point_is_on_wire(QPointF(15, 9.9)));
    }

    TEST_CASE("move_point_by(): Move points while making sure the angles are preserved")
    {
        // Create a wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point(QPointF(0, 80));
        wire->append_point(QPointF(40, 80));
        wire->append_point(QPointF(40, 0));
        wire->append_point(QPointF(20, 0));
        wire->append_point(QPointF(20, 40));
        wire->append_point(QPointF(80, 40));
        manager.add_wire(wire);

        SUBCASE("Move the first point")
        {
            // Move the first point
            wire->move_point_by(0, QVector2D(0, -20));

            // Make sure the first two points moved
            REQUIRE(wire->points().at(0).toPointF() == QPointF(0, 60));
            REQUIRE(wire->points().at(1).toPointF() == QPointF(40, 60));

            // Make sure the other points didn't move
            REQUIRE(wire->points().at(2).toPointF() == QPointF(40, 0));
            REQUIRE(wire->points().at(3).toPointF() == QPointF(20, 0));
            REQUIRE(wire->points().at(4).toPointF() == QPointF(20, 40));
            REQUIRE(wire->points().at(5).toPointF() == QPointF(80, 40));
        }

        SUBCASE("Move a point in the middle of the wire")
        {
            // Move the wire
            wire->move_point_by(4, QVector2D(-20, 20));

            // Make sure the points that are not affected by this haven't moved
            REQUIRE(wire->points().at(0).toPointF() == QPointF(0, 80));
            REQUIRE(wire->points().at(1).toPointF() == QPointF(40, 80));
            REQUIRE(wire->points().at(2).toPointF() == QPointF(40, 0));

            // Make the point and its adjacent points have moved
            REQUIRE(wire->points().at(3).toPointF() == QPointF(0, 0));
            REQUIRE(wire->points().at(4).toPointF() == QPointF(0, 60));
            REQUIRE(wire->points().at(5).toPointF() == QPointF(80, 60));
        }
    }
}