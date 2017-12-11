#include <catch.hpp>
#include <rwe/pathfinding/pathfinding_utils.h>

namespace rwe
{
    TEST_CASE("simplifyPath")
    {
        SECTION("doesn't change a simple two element path")
        {
            std::vector<DiscreteRect> v{
                DiscreteRect(0, 0, 1, 1),
                DiscreteRect(0, 1, 1, 1),
            };

            std::vector<DiscreteRect> expected{
                DiscreteRect(0, 0, 1, 1),
                DiscreteRect(0, 1, 1, 1),
            };

            std::vector<DiscreteRect> actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }

        SECTION("simplifies a straight-line path")
        {
            std::vector<DiscreteRect> v{
                DiscreteRect(0, 0, 1, 1),
                DiscreteRect(0, 1, 1, 1),
                DiscreteRect(0, 2, 1, 1),
                DiscreteRect(0, 3, 1, 1),
            };

            std::vector<DiscreteRect> expected{
                DiscreteRect(0, 0, 1, 1),
                DiscreteRect(0, 3, 1, 1),
            };

            std::vector<DiscreteRect> actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }

        SECTION("simplifies a path with turns")
        {
            std::vector<DiscreteRect> v{
                DiscreteRect(0, 0, 1, 1),

                // down
                DiscreteRect(0, 1, 1, 1),
                DiscreteRect(0, 2, 1, 1),
                DiscreteRect(0, 3, 1, 1),

                // diagonal
                DiscreteRect(1, 4, 1, 1),
                DiscreteRect(2, 5, 1, 1),
                DiscreteRect(3, 6, 1, 1),

                // up
                DiscreteRect(3, 5, 1, 1),

                // right
                DiscreteRect(4, 5, 1, 1),
                DiscreteRect(5, 5, 1, 1),
                DiscreteRect(6, 5, 1, 1),
            };

            std::vector<DiscreteRect> expected{
                DiscreteRect(0, 0, 1, 1),

                // down
                DiscreteRect(0, 3, 1, 1),

                // diagonal
                DiscreteRect(3, 6, 1, 1),

                // up
                DiscreteRect(3, 5, 1, 1),

                // right
                DiscreteRect(6, 5, 1, 1),

            };

            std::vector<DiscreteRect> actual = runSimplifyPath(v);

            REQUIRE(expected == actual);
        }
    }
}
