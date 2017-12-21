#include "EightWayDirection.h"
#include "util.h"

namespace rwe
{
    bool isCardinal(Direction d)
    {
        switch (d)
        {
            case Direction::NORTH:
            case Direction::EAST:
            case Direction::SOUTH:
            case Direction::WEST:
                return true;
            case Direction::NORTHEAST:
            case Direction::SOUTHEAST:
            case Direction::SOUTHWEST:
            case Direction::NORTHWEST:
                return false;
        }

        throw std::runtime_error("Invalid direction");
    }

    bool isDiagonal(Direction d)
    {
        switch (d)
        {
            case Direction::NORTH:
            case Direction::EAST:
            case Direction::SOUTH:
            case Direction::WEST:
                return false;
            case Direction::NORTHEAST:
            case Direction::SOUTHEAST:
            case Direction::SOUTHWEST:
            case Direction::NORTHWEST:
                return true;
        }

        throw std::runtime_error("Invalid direction");
    }

    Point directionToPoint(Direction d)
    {
        switch (d)
        {
            case Direction::NORTH:
                return Point(0, -1);
            case Direction::NORTHEAST:
                return Point(1, -1);
            case Direction::EAST:
                return Point(1, 0);
            case Direction::SOUTHEAST:
                return Point(1, 1);
            case Direction::SOUTH:
                return Point(0, 1);
            case Direction::SOUTHWEST:
                return Point(-1, 1);
            case Direction::WEST:
                return Point(-1, 0);
            case Direction::NORTHWEST:
                return Point(-1, -1);
        }

        throw std::runtime_error("Invalid direction");
    }

    Direction pointToDirection(const Point& p)
    {
        if (p.x == 0 && p.y == -1)
            return Direction::NORTH;
        if (p.x == 1 && p.y == -1)
            return Direction::NORTHEAST;
        if (p.x == 1 && p.y == 0)
            return Direction::EAST;
        if (p.x == 1 && p.y == 1)
            return Direction::SOUTHEAST;
        if (p.x == 0 && p.y == 1)
            return Direction::SOUTH;
        if (p.x == -1 && p.y == 1)
            return Direction::SOUTHWEST;
        if (p.x == -1 && p.y == 0)
            return Direction::WEST;
        if (p.x == -1 && p.y == -1)
            return Direction::NORTHWEST;

        throw std::runtime_error("Invalid point direction");
    }

    unsigned int directionToIndex(Direction d)
    {
        return static_cast<unsigned int>(d);
    }

    Direction directionFromIndex(unsigned int i)
    {
        assert(i < 8);
        return static_cast<Direction>(i);
    }

    unsigned int directionDistance(Direction a, Direction b)
    {
        auto indexDifference = static_cast<int>(directionToIndex(b)) - static_cast<int>(directionToIndex(a));
        return static_cast<unsigned int>(std::abs(wrap(-4, 4, indexDifference)));
    }

    Direction directionFromRadians(float r)
    {
        auto eighth = Pif / 4.0f;

        auto angle = wrap(0.0f, 2.0f * Pif, r + (eighth / 2.0f));

        for (unsigned int i = 0; i < 8; ++i)
        {
            if (angle < (i + 1) * eighth)
            {
                return directionFromIndex(i);
            }
        }

        throw std::logic_error("angle out of range, this should never happen");
    }
}
