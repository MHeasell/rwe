#include <catch.hpp>
#include <rwe/Grid.h>

namespace rwe
{
    TEST_CASE("Grid")
    {
        SECTION("when constructed")
        {
            Grid<int> g(3, 4);

            SECTION("has the given width and height")
            {
                REQUIRE(g.getWidth() == 3);
                REQUIRE(g.getHeight() == 4);
            }

            SECTION("can be written to")
            {
                g.get(0, 0) = 2;
                g.get(2, 3) = 10;
                REQUIRE(g.get(0, 0) == 2);
                REQUIRE(g.get(2, 3) == 10);
            }
        }

        SECTION("zero-argument ctor")
        {
            SECTION("constructs an empty grid")
            {
                Grid<int> g;
                REQUIRE(g.getWidth() == 0);
                REQUIRE(g.getHeight() == 0);
            }
        }

        SECTION("vector ctor")
        {
            SECTION("constructs grid from existing data")
            {
                Grid<int> g(3, 2, {1,2,3,4,5,6});
                REQUIRE(g.get(0, 0) == 1);
                REQUIRE(g.get(1, 0) == 2);
                REQUIRE(g.get(2, 0) == 3);
                REQUIRE(g.get(0, 1) == 4);
                REQUIRE(g.get(1, 1) == 5);
                REQUIRE(g.get(2, 1) == 6);
            }
        }
    }
}
