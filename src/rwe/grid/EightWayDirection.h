#pragma once

#include <array>
#include <rwe/grid/Point.h>

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

    static const std::array<Direction, 4> CardinalDirections = {
        Direction::NORTH,
        Direction::WEST,
        Direction::SOUTH,
        Direction::EAST};

    static const std::array<Direction, 4> DiagonalDirections = {
        Direction::NORTHWEST,
        Direction::SOUTHWEST,
        Direction::SOUTHEAST,
        Direction::NORTHEAST};

    bool isCardinal(Direction d);

    bool isDiagonal(Direction d);

    Point directionToPoint(Direction d);

    Direction pointToDirection(const Point& p);

    unsigned int directionToIndex(Direction d);

    unsigned int directionDistance(Direction a, Direction b);
}
