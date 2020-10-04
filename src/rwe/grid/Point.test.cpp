#include <catch2/catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/grid/Point.h>

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

        SECTION(".maxSingleDimensionDistance")
        {
            SECTION("returns max distance in one dimension between two points")
            {
                REQUIRE(Point(1, 2).maxSingleDimensionDistance(Point(8, 4)) == 7);
            }

            rc::prop("has symmetry", []() {
                auto a = *rc::gen::inRange(-1000, 1000);
                auto b = *rc::gen::inRange(-1000, 1000);
                auto c = *rc::gen::inRange(-1000, 1000);
                auto d = *rc::gen::inRange(-1000, 1000);
                auto result = Point(a, b).maxSingleDimensionDistance(Point(c, d));

                RC_ASSERT(Point(b, a).maxSingleDimensionDistance(Point(d, c)) == result);
                RC_ASSERT(Point(-a, -b).maxSingleDimensionDistance(Point(-c, -d)) == result);
            });
        }
    }
}
