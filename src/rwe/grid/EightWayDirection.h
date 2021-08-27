#pragma once

#include <array>
#include <rwe/grid/Point.h>
#include <rwe/math/rwe_math.h>
#include <stdexcept>

namespace rwe
{
    enum class Direction
    {
        NORTH = 0,
        NORTHWEST,
        WEST,
        SOUTHWEST,
        SOUTH,
        SOUTHEAST,
        EAST,
        NORTHEAST
    };

    static const std::array<Direction, 8> Directions = {
        Direction::NORTH,
        Direction::NORTHWEST,
        Direction::WEST,
        Direction::SOUTHWEST,
        Direction::SOUTH,
        Direction::SOUTHEAST,
        Direction::EAST,
        Direction::NORTHEAST};

    bool isCardinal(Direction d);

    bool isDiagonal(Direction d);

    Point directionToPoint(Direction d);

    Direction pointToDirection(const Point& p);

    int directionToIndex(Direction d);

    int directionDistance(Direction a, Direction b);
}
