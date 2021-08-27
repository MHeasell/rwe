#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>
#include <rwe/grid/DiscreteRect.h>
#include <rwe/pathfinding/OctileDistance_io.h>

namespace rwe
{
    TEST_CASE("DiscreteRect")
    {
        SECTION(".isAdjacentTo")
        {
            SECTION("tests whether points are adjacent")
            {
                //             x, y, w, h
                DiscreteRect r(2, 2, 5, 6);

                //    0 1 2 3 4 5 6 7 8
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
                REQUIRE(!r.isAdjacentTo(1, 0));
                REQUIRE(!r.isAdjacentTo(0, 1));
                REQUIRE(r.isAdjacentTo(1, 1));
                REQUIRE(r.isAdjacentTo(2, 1));
                REQUIRE(r.isAdjacentTo(1, 2));

                // inside top-left
                REQUIRE(!r.isAdjacentTo(2, 2));
                REQUIRE(!r.isAdjacentTo(3, 2));
                REQUIRE(!r.isAdjacentTo(2, 3));

                // bottom-left
                REQUIRE(r.isAdjacentTo(1, 7));
                REQUIRE(r.isAdjacentTo(1, 8));
                REQUIRE(r.isAdjacentTo(2, 8));
                REQUIRE(!r.isAdjacentTo(0, 8));
                REQUIRE(!r.isAdjacentTo(1, 9));

                // inside bottom-left
                REQUIRE(!r.isAdjacentTo(2, 6));
                REQUIRE(!r.isAdjacentTo(2, 7));
                REQUIRE(!r.isAdjacentTo(3, 7));

                // top-right
                REQUIRE(!r.isAdjacentTo(7, 0));
                REQUIRE(!r.isAdjacentTo(8, 1));
                REQUIRE(r.isAdjacentTo(6, 1));
                REQUIRE(r.isAdjacentTo(7, 1));
                REQUIRE(r.isAdjacentTo(7, 2));

                // inside top-right
                REQUIRE(!r.isAdjacentTo(5, 2));
                REQUIRE(!r.isAdjacentTo(6, 2));
                REQUIRE(!r.isAdjacentTo(6, 3));

                // bottom-right
                REQUIRE(!r.isAdjacentTo(7, 9));
                REQUIRE(!r.isAdjacentTo(8, 8));
                REQUIRE(r.isAdjacentTo(6, 8));
                REQUIRE(r.isAdjacentTo(7, 8));
                REQUIRE(r.isAdjacentTo(7, 7));

                // inside bottom-right
                REQUIRE(!r.isAdjacentTo(5, 7));
                REQUIRE(!r.isAdjacentTo(6, 7));
                REQUIRE(!r.isAdjacentTo(6, 6));
            }

            SECTION("tests whether rects are adjacent")
            {
                SECTION("manual tests")
                {
                    DiscreteRect r(4, 2, 5, 6);

                    //    0 1 2 3 4 5 6 7 8 9 a
                    // 0  . . . . . . . . . . .
                    // 1  . . . . . . . . . . .
                    // 2  . . . . x x x x x . .
                    // 3  . . . . x x x x x . .
                    // 4  . . . . x x x x x . .
                    // 5  . . . . x x x x x . .
                    // 6  . . . . x x x x x . .
                    // 7  . . . . x x x x x . .
                    // 8  . . . . . . . . . . .
                    // 9  . . . . . . . . . . .

                    // shares left edge
                    REQUIRE(r.isAdjacentTo(DiscreteRect(1, 3, 3, 2)));

                    // one space away from left edge
                    REQUIRE(!r.isAdjacentTo(DiscreteRect(0, 3, 3, 2)));

                    // shares top-left corner
                    REQUIRE(r.isAdjacentTo(DiscreteRect(2, 0, 2, 2)));

                    // shares right edge
                    REQUIRE(r.isAdjacentTo(DiscreteRect(9, 4, 3, 2)));

                    // one space away from right edge
                    REQUIRE(!r.isAdjacentTo(DiscreteRect(10, 4, 3, 2)));

                    // shares bottom edge
                    REQUIRE(r.isAdjacentTo(DiscreteRect(5, 8, 3, 2)));

                    // one space away from bottom edge
                    REQUIRE(!r.isAdjacentTo(DiscreteRect(5, 9, 3, 2)));

                    // shares top edge
                    REQUIRE(r.isAdjacentTo(DiscreteRect(5, 0, 3, 2)));

                    // one space away from top edge
                    REQUIRE(!r.isAdjacentTo(DiscreteRect(5, 0, 3, 1)));
                }

                rc::prop("a unit rectangle is adjacent if the point is", []()
                {
                    auto x = *rc::gen::positive<int>();
                    auto y = *rc::gen::positive<int>();
                    auto w = *rc::gen::positive<int>();
                    auto h = *rc::gen::positive<int>();
                    auto x2 = *rc::gen::positive<int>();
                    auto y2 = *rc::gen::positive<int>();
                    DiscreteRect r(x, y, w, h);
                    RC_ASSERT(r.isAdjacentTo(DiscreteRect(x2, y2, 1, 1)) == r.isAdjacentTo(x2, y2));
                });
            }
        }

        SECTION("octileDistanceToPerimeter")
        {
            SECTION("finds the shortest distance to the perimeter")
            {
                DiscreteRect r(5, 2, 5, 6);

                //    0 1 2 3 4 5 6 7 8 9 a b
                // 0  . . . . . . . . . . . .
                // 1  . . . . . . . . . . . .
                // 2  . . . . . x x x x x . .
                // 3  . . . . . x x x x x . .
                // 4  . . . . . x x x x x . .
                // 5  . . . . . x x x x x . .
                // 6  . . . . . x x x x x . .
                // 7  . . . . . x x x x x . .
                // 8  . . . . . . . . . . . .
                // 9  . . . . . . . . . . . .

                SECTION("top-left")
                {
                    auto actual = r.octileDistanceToPerimeter(2, 0);
                    auto expected = OctileDistance(1, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("top")
                {
                    auto actual = r.octileDistanceToPerimeter(6, -2);
                    auto expected = OctileDistance(3, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("top-right")
                {
                    auto actual = r.octileDistanceToPerimeter(11, 0);
                    auto expected = OctileDistance(0, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("right")
                {
                    auto actual = r.octileDistanceToPerimeter(11, 4);
                    auto expected = OctileDistance(1, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("bottom-right")
                {
                    auto actual = r.octileDistanceToPerimeter(11, 9);
                    auto expected = OctileDistance(0, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("bottom")
                {
                    auto actual = r.octileDistanceToPerimeter(9, 10);
                    auto expected = OctileDistance(2, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("bottom-left")
                {
                    auto actual = r.octileDistanceToPerimeter(3, 10);
                    auto expected = OctileDistance(1, 1);
                    REQUIRE(actual == expected);
                }

                SECTION("left")
                {
                    auto actual = r.octileDistanceToPerimeter(0, 3);
                    auto expected = OctileDistance(4, 0);
                    REQUIRE(actual == expected);
                }


                SECTION("inside-left")
                {
                    auto actual = r.octileDistanceToPerimeter(5, 5);
                    auto expected = OctileDistance(1, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("inside-top")
                {
                    auto actual = r.octileDistanceToPerimeter(7, 3);
                    auto expected = OctileDistance(2, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("inside-right")
                {
                    auto actual = r.octileDistanceToPerimeter(9, 6);
                    auto expected = OctileDistance(1, 0);
                    REQUIRE(actual == expected);
                }

                SECTION("inside-bottom")
                {
                    auto actual = r.octileDistanceToPerimeter(8, 7);
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

        SECTION("::fromPoints")
        {
            SECTION("creates a rect from internal points")
            {
                auto actual = DiscreteRect::fromPoints(Point(1, 2), Point(5, 4));
                REQUIRE(actual == DiscreteRect(1, 2, 5, 3));
            }

            SECTION("works when points are flipped")
            {
                auto actual = DiscreteRect::fromPoints(Point(5, 4), Point(1, 2));
                REQUIRE(actual == DiscreteRect(1, 2, 5, 3));
            }
        }

        SECTION(".contains")
        {
            SECTION("returns true when point is in the rectangle")
            {
                REQUIRE(!DiscreteRect(2, 3, 4, 5).contains(Point(1, 3)));
                REQUIRE(DiscreteRect(2, 3, 4, 5).contains(Point(2, 3)));
                REQUIRE(DiscreteRect(2, 3, 4, 5).contains(Point(5, 3)));
                REQUIRE(!DiscreteRect(2, 3, 4, 5).contains(Point(6, 3)));

                REQUIRE(!DiscreteRect(2, 3, 4, 5).contains(Point(2, 2)));
                REQUIRE(DiscreteRect(2, 3, 4, 5).contains(Point(2, 3)));
                REQUIRE(DiscreteRect(2, 3, 4, 5).contains(Point(2, 7)));
                REQUIRE(!DiscreteRect(2, 3, 4, 5).contains(Point(2, 8)));
            }
        }

        SECTION(".translate")
        {
            SECTION("translates the rectangle")
            {
                REQUIRE(DiscreteRect(2, 3, 4, 5).translate(3, 6) == DiscreteRect(5, 9, 4, 5));
                REQUIRE(DiscreteRect(2, 3, 4, 5).translate(-1, -3) == DiscreteRect(1, 0, 4, 5));
            }
        }
    }
}
