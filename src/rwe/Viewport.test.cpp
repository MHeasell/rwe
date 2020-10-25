#include <catch2/catch.hpp>
#include <rwe/Viewport.h>

namespace rwe
{
    TEST_CASE("ViewportService")
    {
        SECTION("toClipSpace")
        {
            SECTION("works in a simple case")
            {
                Viewport vp(0, 0, 8, 6);
                auto v = vp.toClipSpace(2, 2);
                REQUIRE(v.x == -0.5f);
                REQUIRE(v.y == Approx(0.33333333));
            }
        }

        SECTION("toViewportSpace")
        {
            SECTION("works in a simple case")
            {
                Viewport vp(0, 0, 8, 6);
                auto p = vp.toViewportSpace(-0.5f, 1.0f / 3.0f);
                REQUIRE(p.x == 2);
                REQUIRE(p.y == 2);
            }
        }

        SECTION("toOtherViewport")
        {
            SECTION("works in a simple case")
            {
                Viewport va(1, 2, 3, 4);
                Viewport vb(3, 4, 7, 9);

                auto p = va.toOtherViewport(vb, 2, 3);
                REQUIRE(p.x == 0);
                REQUIRE(p.y == 1);
            }
        }
    }
}
