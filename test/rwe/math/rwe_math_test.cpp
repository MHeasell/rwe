#include <catch.hpp>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    TEST_CASE("convertScreenToClipSpace")
    {
        SECTION("works in a simple case")
        {
            auto v = convertScreenToClipSpace(8, 6, Point(2, 2));
            REQUIRE(v.x == -0.5f);
            REQUIRE(v.y == Approx(0.33333333));
        }
    }

    TEST_CASE("wrap")
    {
        SECTION("wraps values")
        {
            REQUIRE(wrap(1.0f, 3.0f, 2.0f) == 2.0f);
            REQUIRE(wrap(1.0f, 3.0f, 3.5f) == 1.5f);
            REQUIRE(wrap(1.0f, 3.0f, 0.5f) == 2.5f);

            REQUIRE(wrap(-2.0f, 5.0f, -1.0f) == -1.0f);
            REQUIRE(wrap(-2.0f, 5.0f, -3.0f) == 4.0f);
        }
    }
}
