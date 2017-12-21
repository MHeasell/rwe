#include <catch.hpp>
#include <rwe/Point.h>

namespace rwe
{
    TEST_CASE("Point")
    {
        SECTION("can be checked for equality")
        {
            REQUIRE(Point(1, 2) == Point(1, 2));
            REQUIRE(!(Point(1, 2) != Point(1, 2)));

            REQUIRE(Point(1, 2) != Point(1, 3));
            REQUIRE(!(Point(1, 2) == Point(1, 3)));

            REQUIRE(Point(1, 2) != Point(0, 2));
            REQUIRE(!(Point(1, 2) == Point(0, 2)));
        }

        SECTION("can be added together")
        {
            REQUIRE(Point(1, 2) + Point(3, 4) == Point(4, 6));
        }

        SECTION("can be subtracted")
        {
            REQUIRE(Point(1, 2) - Point(3, 4) == Point(-2, -2));
        }
    }
}
