#include <catch.hpp>
#include <rwe/DiscreteRect.h>
#include <rwe/pathfinding/OctileDistance_io.h>

namespace rwe
{
    TEST_CASE("DiscreteRect")
    {
        SECTION(".isAdjacentTo")
        {
            SECTION("tests whether points are adjacent")
            {
                DiscreteRect r(1, 2, 5, 6);

                //   -1 0 1 2 3 4 5 6 7
                // 0  . . . . . . . . .
                // 1  . . . . . . . . .
                // 2  . . x x x x x . .
                // 3  . . x x x x x . .
                // 4  . . x x x x x . .
                // 5  . . x x x x x . .
                // 6  . . x x x x x . .
                // 7  . . x x x x x . .
                // 8  . . . . . . . . .
                // 9  . . . . . . . . .

                // outside top-left
                REQUIRE(!r.isAdjacentTo(0, 0));
                REQUIRE(!r.isAdjacentTo(-1, 1));
                REQUIRE(r.isAdjacentTo(0, 1));
                REQUIRE(r.isAdjacentTo(1, 1));
                REQUIRE(r.isAdjacentTo(0, 2));

                // inside top-left
                REQUIRE(!r.isAdjacentTo(1, 2));
                REQUIRE(!r.isAdjacentTo(2, 2));
                REQUIRE(!r.isAdjacentTo(1, 3));

                // bottom-left
                REQUIRE(r.isAdjacentTo(0, 7));
                REQUIRE(r.isAdjacentTo(0, 8));
                REQUIRE(r.isAdjacentTo(1, 8));
                REQUIRE(!r.isAdjacentTo(-1, 8));
                REQUIRE(!r.isAdjacentTo(0, 9));

                // inside bottom-left
                REQUIRE(!r.isAdjacentTo(1, 6));
                REQUIRE(!r.isAdjacentTo(1, 7));
                REQUIRE(!r.isAdjacentTo(2, 7));

                // top-right
                REQUIRE(!r.isAdjacentTo(6, 0));
                REQUIRE(!r.isAdjacentTo(7, 1));
                REQUIRE(r.isAdjacentTo(5, 1));
                REQUIRE(r.isAdjacentTo(6, 1));
                REQUIRE(r.isAdjacentTo(6, 2));

                // inside top-right
                REQUIRE(!r.isAdjacentTo(4, 2));
                REQUIRE(!r.isAdjacentTo(5, 2));
                REQUIRE(!r.isAdjacentTo(5, 3));

                // bottom-right
                REQUIRE(!r.isAdjacentTo(6, 9));
                REQUIRE(!r.isAdjacentTo(7, 8));
                REQUIRE(r.isAdjacentTo(5, 8));
                REQUIRE(r.isAdjacentTo(6, 8));
                REQUIRE(r.isAdjacentTo(6, 7));

                // inside bottom-right
                REQUIRE(!r.isAdjacentTo(4, 7));
                REQUIRE(!r.isAdjacentTo(5, 7));
                REQUIRE(!r.isAdjacentTo(5, 6));
            }
        }

        SECTION("octileDistanceToPerimeter")
        {
            SECTION("finds the shortest distance to the perimeter")
            {
                DiscreteRect r(1, 2, 5, 6);

                //   -1 0 1 2 3 4 5 6 7
                // 0  . . . . . . . . .
                // 1  . . . . . . . . .
                // 2  . . x x x x x . .
                // 3  . . x x x x x . .
                // 4  . . x x x x x . .
                // 5  . . x x x x x . .
                // 6  . . x x x x x . .
                // 7  . . x x x x x . .
                // 8  . . . . . . . . .
                // 9  . . . . . . . . .

                SECTION("top-left")
                {
                    auto actual = r.octileDistanceToPerimeter(-2, 0);
                    auto expected = OctileDistance(1, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("top")
                {
                    auto actual = r.octileDistanceToPerimeter(2, -2);
                    auto expected = OctileDistance(3, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("top-right")
                {
                    auto actual = r.octileDistanceToPerimeter(7, 0);
                    auto expected = OctileDistance(0, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("right")
                {
                    auto actual = r.octileDistanceToPerimeter(7, 4);
                    auto expected = OctileDistance(1, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("bottom-right")
                {
                    auto actual = r.octileDistanceToPerimeter(7, 9);
                    auto expected = OctileDistance(0, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("bottom")
                {
                    auto actual = r.octileDistanceToPerimeter(5, 10);
                    auto expected = OctileDistance(2, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("bottom-left")
                {
                    auto actual = r.octileDistanceToPerimeter(-1, 10);
                    auto expected = OctileDistance(1, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("left")
                {
                    auto actual = r.octileDistanceToPerimeter(-4, 3);
                    auto expected = OctileDistance(4, 0);
                    REQUIRE(actual == expected);
                }


                SECTION("inside-left")
                {
                    auto actual = r.octileDistanceToPerimeter(1, 5);
                    auto expected = OctileDistance(1, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("inside-top")
                {
                    auto actual = r.octileDistanceToPerimeter(3, 3);
                    auto expected = OctileDistance(2, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("inside-right")
                {
                    auto actual = r.octileDistanceToPerimeter(5, 6);
                    auto expected = OctileDistance(1, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("inside-bottom")
                {
                    auto actual = r.octileDistanceToPerimeter(4, 7);
                    auto expected = OctileDistance(1, 0);
                    REQUIRE(actual == expected);
                }
            }
        }

        SECTION(".intersection")
        {
            SECTION("returns the intersection of rectangles")
            {
                // . . . . . . . . . .
                // . . . . . . . . . .
                // . a a a . . . . . .
                // . a c c b b b b . .
                // . a c c b b b b . .
                // . a a a . . . . . .
                // . . . . . . . . . .
                DiscreteRect a(1, 2, 3, 4);
                DiscreteRect b(2, 3, 6, 2);
                DiscreteRect expected(2, 3, 2, 2);
                REQUIRE(a.intersection(b) == expected);
            }
            SECTION("returns None if rectangles do not intersect")
            {
                DiscreteRect a(2, 2, 2, 2);
                DiscreteRect b(4, 4, 2, 2);
                REQUIRE(a.intersection(b) == std::nullopt);
            }
        }
    }
}
