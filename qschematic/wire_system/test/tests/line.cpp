#include "../3rdparty/doctest.h"
#include "../../line.h"

TEST_SUITE("Line")
{
    TEST_CASE("is_null()")
    {
        wire_system::line line(QPointF(10, 20), QPointF(10, 20));

        REQUIRE(line.is_null());
        REQUIRE(line.is_horizontal());
        REQUIRE(line.is_vertical());
    }

    TEST_CASE("is_horizontal()")
    {
        wire_system::line line(QPointF(10, 20), QPointF(-12.5, 20));

        REQUIRE_FALSE(line.is_null());
        REQUIRE(line.is_horizontal());
        REQUIRE_FALSE(line.is_vertical());
    }

    TEST_CASE("is_vertical()")
    {
        wire_system::line line(QPointF(10, 20), QPointF(10, 1000));

        REQUIRE_FALSE(line.is_null());
        REQUIRE(line.is_vertical());
        REQUIRE_FALSE(line.is_horizontal());
    }

    TEST_CASE("lenght()")
    {
        SUBCASE("line 1")
        {
            wire_system::line line(QPointF(0, 0), QPointF(0, 10));
            REQUIRE(line.lenght() == 10);
        }

        SUBCASE("line 2")
        {
            wire_system::line line(QPointF(0, 0), QPointF(0, -10));
            REQUIRE(line.lenght() == 10);
        }

        SUBCASE("line 3")
        {
            wire_system::line line(QPointF(12.534, -123.643), QPointF(62.3, 15.535));
            REQUIRE(doctest::Approx(line.lenght()).epsilon(0.01) == 147.8);
        }
    }
}