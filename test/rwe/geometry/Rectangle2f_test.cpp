#include <catch.hpp>
#include <rwe/geometry/Rectangle2f.h>

namespace rwe
{
    TEST_CASE("Rectangle2f")
    {
        SECTION(".contains")
        {
            SECTION("returns true for points inside the rectangle")
            {
                Rectangle2f r(1.0f, 2.0f, 3.0f, 4.0f);
                REQUIRE(r.contains(0.0f, 0.0f));
                REQUIRE(r.contains(-1.8f, -1.8f));
                REQUIRE(r.contains(3.8f, 5.8f));
            }

            SECTION("returns false for points outside the rectangle")
            {
                Rectangle2f r(1.0f, 2.0f, 3.0f, 4.0f);
                REQUIRE(!r.contains(-2.1f, -2.1f));
                REQUIRE(!r.contains(4.1f, 6.1f));
            }

            SECTION("returns true for points on the edge")
            {
                Rectangle2f r(1.0f, 2.0f, 3.0f, 4.0f);
                REQUIRE(r.contains(-2.0f, -2.0f));
                REQUIRE(r.contains(4.0f, 6.0f));
            }
        }

        SECTION("::fromTLBR")
        {
            SECTION("constructs a rectangle from points")
            {
                auto r = Rectangle2f::fromTLBR(1.0f, 3.0f, 4.0f, 7.0f);
                REQUIRE(r.position.x == 2.5f);
                REQUIRE(r.position.y == 5.0f);
                REQUIRE(r.extents.x == 3.0f);
                REQUIRE(r.extents.y == 4.0f);
            }
        }
    }
}
