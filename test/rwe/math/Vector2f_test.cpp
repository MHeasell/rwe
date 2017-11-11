#include <catch.hpp>
#include <rwe/math/Vector2f.h>
#include <rwe/util.h>

namespace rwe
{
    TEST_CASE("Vector2f")
    {
        SECTION("angleTo")
        {
            SECTION("works for parallel vectors")
            {
                Vector2f a(1.0f, 0.0f);
                Vector2f b(1.0f, 0.0f);
                REQUIRE(a.angleTo(b) == Approx(0.0f));
            }
            SECTION("works for perpendicular vectors")
            {
                Vector2f a(1.0f, 0.0f);
                Vector2f b(0.0f, 1.0f);
                REQUIRE(a.angleTo(b) == Approx(Pif / 2.0f));
            }
            SECTION("works for perpendicular vectors with negative angle")
            {
                Vector2f a(0.0f, 1.0f);
                Vector2f b(1.0f, 0.0f);
                REQUIRE(a.angleTo(b) == Approx(-Pif / 2.0f));
            }
            SECTION("works for opposite vectors")
            {
                Vector2f a(0.0f, 1.0f);
                Vector2f b(0.0f, -1.0f);
                REQUIRE(a.angleTo(b) == Approx(-Pif));
            }
        }
    }
}
