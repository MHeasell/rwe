#include <catch2/catch.hpp>
#include <rwe/unit_util.h>

namespace rwe
{
    TEST_CASE("removeFromBuildQueue")
    {
        SECTION("Removes nothing from the empty list")
        {
            std::deque<std::pair<std::string, int>> buildQueue;

            removeFromBuildQueue(buildQueue, "ARMPW", 1);

            std::deque<std::pair<std::string, int>> expected;
            REQUIRE(buildQueue == expected);
        }

        SECTION("Removes the only element in a trivial case")
        {
            std::deque<std::pair<std::string, int>> buildQueue{
                std::pair{"ARMPW", 1}
            };

            removeFromBuildQueue(buildQueue, "ARMPW", 1);

            std::deque<std::pair<std::string, int>> expected;
            REQUIRE(buildQueue == expected);
        }

        SECTION("Removes nothing when count is 0")
        {
            std::deque<std::pair<std::string, int>> buildQueue{
                std::pair{"ARMPW", 1}
            };

            removeFromBuildQueue(buildQueue, "ARMPW", 0);

            std::deque<std::pair<std::string, int>> expected{
                std::pair{"ARMPW", 1}
            };
            REQUIRE(buildQueue == expected);
        }

        SECTION("Doesn't break when removing more than there are in the list")
        {
            std::deque<std::pair<std::string, int>> buildQueue{
                std::pair{"ARMPW", 1}
            };

            removeFromBuildQueue(buildQueue, "ARMPW", 2);

            std::deque<std::pair<std::string, int>> expected;
            REQUIRE(buildQueue == expected);
        }

        SECTION("Leaves excess elements there")
        {
            std::deque<std::pair<std::string, int>> buildQueue{
                std::pair{"ARMPW", 8}
            };

            removeFromBuildQueue(buildQueue, "ARMPW", 3);

            std::deque<std::pair<std::string, int>> expected{
                std::pair{"ARMPW", 5}
            };
            REQUIRE(buildQueue == expected);
        }

        SECTION("Removes consecutive elements from the back")
        {
            std::deque<std::pair<std::string, int>> buildQueue{
                std::pair{"ARMPW", 1},
                std::pair{"ARMPW", 5},
                std::pair{"ARMPW", 3}
            };

            removeFromBuildQueue(buildQueue, "ARMPW", 4);

            std::deque<std::pair<std::string, int>> expected{
                std::pair{"ARMPW", 1},
                std::pair{"ARMPW", 4}
            };
            REQUIRE(buildQueue == expected);
        }

        SECTION("Ignores non-matching elements")
        {
            std::deque<std::pair<std::string, int>> buildQueue{
                std::pair{"ARMPW", 1},
                std::pair{"ARMPW", 5},
                std::pair{"ARMCK", 7},
                std::pair{"ARMPW", 3}
            };

            removeFromBuildQueue(buildQueue, "ARMPW", 4);

            std::deque<std::pair<std::string, int>> expected{
                std::pair{"ARMPW", 1},
                std::pair{"ARMPW", 4},
                std::pair{"ARMCK", 7}
            };
            REQUIRE(buildQueue == expected);
        }
    }

    TEST_CASE("getBuildQueueTotalsStatic")
    {
        SECTION("gets totals from build queue")
        {
            std::deque<std::pair<std::string, int>> buildQueue{
                std::pair{"ARMPW", 1},
                std::pair{"ARMPW", 5},
                std::pair{"ARMCK", 7},
                std::pair{"ARMPW", 3}
            };

            std::unordered_map<std::string, int> expected{
                {"ARMPW", 9},
                {"ARMCK", 7}
            };

            REQUIRE(getBuildQueueTotalsStatic(buildQueue) == expected);
        }
    }
}
