#include <catch.hpp>
#include <rwe/pathfinding/pathfinding_utils.h>

namespace rwe
{
    TEST_CASE("simplifyPath")
    {
        SECTION("doesn't change a simple two element path")
        {
            std::vector<PathVertex> v{
                PathVertex(Point(0, 0), Direction::SOUTH),
                PathVertex(Point(0, 1), Direction::SOUTH),
            };

            std::vector<PathVertex> expected{
                PathVertex(Point(0, 0), Direction::SOUTH),
                PathVertex(Point(0, 1), Direction::SOUTH),
            };

            auto actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }

        SECTION("simplifies a straight-line path")
        {
            std::vector<PathVertex> v{
                PathVertex(Point(0, 0), Direction::SOUTH),
                PathVertex(Point(0, 1), Direction::SOUTH),
                PathVertex(Point(0, 2), Direction::SOUTH),
                PathVertex(Point(0, 3), Direction::SOUTH),
            };

            std::vector<PathVertex> expected{
                PathVertex(Point(0, 0), Direction::SOUTH),
                PathVertex(Point(0, 3), Direction::SOUTH),
            };

            auto actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }

        SECTION("simplifies a path with turns")
        {
            std::vector<PathVertex> v{
                PathVertex(Point(0, 0), Direction::SOUTH),

                // down
                PathVertex(Point(0, 1), Direction::SOUTH),
                PathVertex(Point(0, 2), Direction::SOUTH),
                PathVertex(Point(0, 3), Direction::SOUTH),

                // diagonal
                PathVertex(Point(1, 4), Direction::SOUTHEAST),
                PathVertex(Point(2, 5), Direction::SOUTHEAST),
                PathVertex(Point(3, 6), Direction::SOUTHEAST),

                // up
                PathVertex(Point(3, 5), Direction::NORTH),

                // right
                PathVertex(Point(4, 5), Direction::EAST),
                PathVertex(Point(5, 5), Direction::EAST),
                PathVertex(Point(6, 5), Direction::EAST),
            };

            std::vector<PathVertex> expected{
                PathVertex(Point(0, 0), Direction::SOUTH),

                // down
                PathVertex(Point(0, 3), Direction::SOUTH),

                // diagonal
                PathVertex(Point(3, 6), Direction::SOUTHEAST),

                // up
                PathVertex(Point(3, 5), Direction::NORTH),

                // right
                PathVertex(Point(6, 5), Direction::EAST),

            };

            auto actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }
    }
}
