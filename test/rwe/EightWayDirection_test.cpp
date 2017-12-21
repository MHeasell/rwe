#include <catch.hpp>
#include <rwe/EightWayDirection.h>
#include <rwe/util.h>

namespace rwe
{
    TEST_CASE("Direction")
    {
        SECTION("isCardinal")
        {
            SECTION("is true for cardinal directions")
            {
                REQUIRE(isCardinal(Direction::EAST));
                REQUIRE(isCardinal(Direction::WEST));
                REQUIRE(isCardinal(Direction::NORTH));
                REQUIRE(isCardinal(Direction::SOUTH));

                REQUIRE(!isCardinal(Direction::NORTHEAST));
                REQUIRE(!isCardinal(Direction::NORTHWEST));
                REQUIRE(!isCardinal(Direction::SOUTHEAST));
                REQUIRE(!isCardinal(Direction::SOUTHWEST));
            }
        }

        SECTION("isDiagonal")
        {
            SECTION("is true for diagonal directions")
            {
                REQUIRE(isDiagonal(Direction::NORTHEAST));
                REQUIRE(isDiagonal(Direction::NORTHWEST));
                REQUIRE(isDiagonal(Direction::SOUTHEAST));
                REQUIRE(isDiagonal(Direction::SOUTHWEST));

                REQUIRE(!isDiagonal(Direction::EAST));
                REQUIRE(!isDiagonal(Direction::WEST));
                REQUIRE(!isDiagonal(Direction::NORTH));
                REQUIRE(!isDiagonal(Direction::SOUTH));
            }
        }

        SECTION("pointToDirection")
        {
            SECTION("combines with directionToPoint to the identity")
            {
                for (auto d : Directions)
                {
                    REQUIRE(pointToDirection(directionToPoint(d)) == d);
                }
            }
        }

        SECTION("directionToIndex")
        {
            SECTION("returns an index")
            {
                REQUIRE(directionToIndex(Direction::NORTH) == 0);
                REQUIRE(directionToIndex(Direction::NORTHWEST) == 1);
                REQUIRE(directionToIndex(Direction::WEST) == 2);
                REQUIRE(directionToIndex(Direction::SOUTHWEST) == 3);
                REQUIRE(directionToIndex(Direction::SOUTH) == 4);
                REQUIRE(directionToIndex(Direction::SOUTHEAST) == 5);
                REQUIRE(directionToIndex(Direction::EAST) == 6);
                REQUIRE(directionToIndex(Direction::NORTHEAST) == 7);
            }
        }

        SECTION("directionDistance")
        {
            SECTION("returns 0 for a direction and itself")
            {
                for (auto d : Directions)
                {
                    REQUIRE(directionDistance(d, d) == 0);
                }
            }

            SECTION("returns index-distance between directions")
            {
                REQUIRE(directionDistance(Direction::NORTH, Direction::EAST) == 2);
                REQUIRE(directionDistance(Direction::WEST, Direction::NORTHEAST) == 3);
                REQUIRE(directionDistance(Direction::NORTHEAST, Direction::WEST) == 3);
            }
        }

        SECTION("directionFromRadians")
        {
            SECTION("converts an angle to a direction")
            {
                REQUIRE(directionFromRadians((0.0f / 4.0f) * Pif) == Direction::NORTH);
                REQUIRE(directionFromRadians((1.0f / 4.0f) * Pif) == Direction::NORTHWEST);
                REQUIRE(directionFromRadians((2.0f / 4.0f) * Pif) == Direction::WEST);
                REQUIRE(directionFromRadians((3.0f / 4.0f) * Pif) == Direction::SOUTHWEST);
                REQUIRE(directionFromRadians((4.0f / 4.0f) * Pif) == Direction::SOUTH);
                REQUIRE(directionFromRadians((5.0f / 4.0f) * Pif) == Direction::SOUTHEAST);
                REQUIRE(directionFromRadians((6.0f / 4.0f) * Pif) == Direction::EAST);
                REQUIRE(directionFromRadians((7.0f / 4.0f) * Pif) == Direction::NORTHEAST);
                REQUIRE(directionFromRadians((8.0f / 4.0f) * Pif) == Direction::NORTH);
            }
        }
    }
}
