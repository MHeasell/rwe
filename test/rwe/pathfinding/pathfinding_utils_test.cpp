#include <catch.hpp>
#include <rwe/pathfinding/pathfinding_utils.h>

namespace rwe
{
    TEST_CASE("simplifyPath")
    {
        SECTION("doesn't change a simple two element path")
        {
            std::vector<Point> v{
                Point(0, 0),
                Point(0, 1),
            };

            std::vector<Point> expected{
                Point(0, 0),
                Point(0, 1),
            };

            auto actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }

        SECTION("simplifies a straight-line path")
        {
            std::vector<Point> v{
                Point(0, 0),
                Point(0, 1),
                Point(0, 2),
                Point(0, 3),
            };

            std::vector<Point> expected{
                Point(0, 0),
                Point(0, 3),
            };

            auto actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }

        SECTION("simplifies a path with turns")
        {
            std::vector<Point> v{
                Point(0, 0),

                // down
                Point(0, 1),
                Point(0, 2),
                Point(0, 3),

                // diagonal
                Point(1, 4),
                Point(2, 5),
                Point(3, 6),

                // up
                Point(3, 5),

                // right
                Point(4, 5),
                Point(5, 5),
                Point(6, 5),
            };

            std::vector<Point> expected{
                Point(0, 0),

                // down
                Point(0, 3),

                // diagonal
                Point(3, 6),

                // up
                Point(3, 5),

                // right
                Point(6, 5),

            };

            auto actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }
    }

    TEST_CASE("octileDistance")
    {
        SECTION("returns octile distance between points")
        {
            // horizontal
            REQUIRE(octileDistance(Point(2, 4), Point(2, 11)) == OctileDistance(7, 0));
            REQUIRE(octileDistance(Point(2, 4), Point(2, -2)) == OctileDistance(6, 0));

            // vertical
            REQUIRE(octileDistance(Point(2, 4), Point(5, 4)) == OctileDistance(3, 0));
            REQUIRE(octileDistance(Point(2, 4), Point(0, 4)) == OctileDistance(2, 0));

            // pure diagonal
            REQUIRE(octileDistance(Point(2, 4), Point(4, 6)) == OctileDistance(0, 2));
            REQUIRE(octileDistance(Point(2, 4), Point(4, 2)) == OctileDistance(0, 2));
            REQUIRE(octileDistance(Point(2, 4), Point(0, 6)) == OctileDistance(0, 2));
            REQUIRE(octileDistance(Point(2, 4), Point(0, 2)) == OctileDistance(0, 2));

            // combo
            REQUIRE(octileDistance(Point(2, 4), Point(5, 6)) == OctileDistance(1, 2));
        }
    }
}
