#include "../3rdparty/doctest.h"
#include "../../line.hpp"

TEST_SUITE("Line")
{
    TEST_CASE("is_null()")
    {
        wire_system::line line(QPointF(10, 20), QPointF(10, 20));

        CHECK(line.is_null());
        CHECK(line.is_horizontal());
        CHECK(line.is_vertical());
    }

    TEST_CASE("is_horizontal()")
    {
        wire_system::line line(QPointF(10, 20), QPointF(-12.5, 20));

        CHECK_FALSE(line.is_null());
        CHECK(line.is_horizontal());
        CHECK_FALSE(line.is_vertical());
    }

    TEST_CASE("is_vertical()")
    {
        wire_system::line line(QPointF(10, 20), QPointF(10, 1000));

        CHECK_FALSE(line.is_null());
        CHECK(line.is_vertical());
        CHECK_FALSE(line.is_horizontal());
    }

    TEST_CASE("length()")
    {
        SUBCASE("line 1")
        {
            wire_system::line line(QPointF(0, 0), QPointF(0, 10));
            CHECK_EQ(line.length(), doctest::Approx(10));
        }

        SUBCASE("line 2")
        {
            wire_system::line line(QPointF(0, 0), QPointF(0, -10));
            CHECK_EQ(line.length(), doctest::Approx(10));
        }

        SUBCASE("line 3")
        {
            wire_system::line line(QPointF(12.534, -123.643), QPointF(62.3, 15.535));
            CHECK_EQ(line.length(), doctest::Approx(147.8).epsilon(0.01));
        }
    }
}