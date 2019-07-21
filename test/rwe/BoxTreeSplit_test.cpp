#include <catch2/catch.hpp>
#include <rwe/BoxTreeSplit.h>

namespace rwe
{
    TEST_CASE("BoxTreeSplit")
    {
        SECTION("packGrids")
        {
            SECTION("works for a simple case")
            {
                Grid<int> a(4, 3);
                Grid<int> b(8, 6);

                std::vector<Grid<int>*> vec{&a, &b};

                auto output = packGrids(vec);

                REQUIRE(output.width == 8);
                REQUIRE(output.height == 9);

                REQUIRE(output.entries.size() == 2);
                REQUIRE(output.entries[0].x == 0);
                REQUIRE(output.entries[0].y == 0);
                REQUIRE(output.entries[0].value == &b);

                REQUIRE(output.entries[1].x == 0);
                REQUIRE(output.entries[1].y == 6);
                REQUIRE(output.entries[1].value == &a);
            }

            SECTION("works in both dimensions")
            {
                std::vector<Grid<int>> v{
                    Grid<int>(4, 4),
                    Grid<int>(4, 4),
                    Grid<int>(4, 4),
                    Grid<int>(4, 4),
                };

                std::vector<Grid<int>*> vec;
                for (auto& e : v)
                {
                    vec.push_back(&e);
                }

                auto output = packGrids(vec);

                REQUIRE(output.width == 8);
                REQUIRE(output.height == 8);
            }
        }
    }
}
