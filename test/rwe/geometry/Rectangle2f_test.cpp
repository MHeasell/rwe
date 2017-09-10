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
                REQUIRE(r.position.x == 5.0f);
                REQUIRE(r.position.y == 2.5f);
                REQUIRE(r.extents.x == 2.0f);
                REQUIRE(r.extents.y == 1.5f);
            }
        }

        SECTION("::fromTopLeft")
        {
            SECTION("constructs a rectangle from points")
            {
                auto r = Rectangle2f::fromTopLeft(1.0f, 3.0f, 4.0f, 7.0f);
                REQUIRE(r.position.x == 3.0f);
                REQUIRE(r.position.y == 6.5f);
                REQUIRE(r.extents.x == 2.0f);
                REQUIRE(r.extents.y == 3.5f);
            }
        }
        SECTION("getters")
        {
            SECTION("get various properties")
            {
                auto r = Rectangle2f::fromTLBR(1.0f, 3.0f, 4.0f, 7.0f);

                REQUIRE(r.width() == 4.0f);
                REQUIRE(r.height() == 3.0f);

                REQUIRE(r.top() == 1.0f);
                REQUIRE(r.bottom() == 4.0f);
                REQUIRE(r.left() == 3.0f);
                REQUIRE(r.right() == 7.0f);

                REQUIRE(r.topLeft() == Vector2f(3.0f, 1.0f));
                REQUIRE(r.topRight() == Vector2f(7.0f, 1.0f));
                REQUIRE(r.bottomLeft() == Vector2f(3.0f, 4.0f));
                REQUIRE(r.bottomRight() == Vector2f(7.0f, 4.0f));
            }
        }

        SECTION("operator==")
        {
            SECTION("tests equality")
            {
                auto a = Rectangle2f(1.0f, 3.0f, 4.0f, 7.0f);
                auto b = Rectangle2f(1.0f, 3.0f, 4.0f, 7.0f);

                // each of these has one element changed
                auto c = Rectangle2f(2.0f, 3.0f, 4.0f, 7.0f);
                auto d = Rectangle2f(1.0f, 4.0f, 4.0f, 7.0f);
                auto e = Rectangle2f(1.0f, 3.0f, 5.0f, 7.0f);
                auto f = Rectangle2f(1.0f, 3.0f, 4.0f, 8.0f);

                REQUIRE(a == b);
                REQUIRE(!(a != b));

                REQUIRE(a != c);
                REQUIRE(!(a == c));

                REQUIRE(a != d);
                REQUIRE(!(a == d));

                REQUIRE(a != e);
                REQUIRE(!(a == e));

                REQUIRE(a != f);
                REQUIRE(!(a == f));
            }
        }
    }
}
