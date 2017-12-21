#ifndef RWE_EIGHTWAYDIRECTION_H
#define RWE_EIGHTWAYDIRECTION_H

#include "Point.h"
#include <array>
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

    unsigned int directionToIndex(Direction d);

    unsigned int directionDistance(Direction a, Direction b);

    Direction directionFromRadians(float r);
}

#endif
