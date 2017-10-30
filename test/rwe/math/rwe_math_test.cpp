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
}
