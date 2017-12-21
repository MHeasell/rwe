#ifndef RWE_PATHFINDING_UTILS_H
#define RWE_PATHFINDING_UTILS_H

#include <rwe/DiscreteRect.h>
#include <rwe/EightWayDirection.h>
#include <rwe/Point.h>

namespace rwe
{
    struct PathVertex
    {
        Point position;
        Direction direction;

        PathVertex() = default;
        PathVertex(const Point& position, Direction direction) : position(position), direction(direction)
        {
        }

        bool operator==(const PathVertex& rhs) const
        {
            return position == rhs.position && direction == rhs.direction;
        }

        bool operator!=(const PathVertex& rhs) const
        {
            return !(rhs == *this);
        }
    };

    struct PathCost
    {
        float distance{0.0f};
        unsigned int turningDistance{0};
        bool operator<(const PathCost& rhs) const
        {
            if (almostEquals(distance, rhs.distance, 1.0f))
            {
                return turningDistance < rhs.turningDistance;
            }

            return distance < rhs.distance;
        }

        PathCost operator+(const PathCost& rhs) const
        {
            return PathCost{distance + rhs.distance, turningDistance + rhs.turningDistance};
        }
    };
}

namespace std
{
    template <>
    struct hash<rwe::PathVertex>
    {
        std::size_t operator()(const rwe::PathVertex& v) const noexcept
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, v.position);
            boost::hash_combine(seed, v.direction);
            return seed;
        }
    };
}

namespace rwe
{
    /**
     * This method assumes that the input path has a minimum of one element.
     * If this is not the case, the behaviour is undefined.
     */
    template <typename It, typename Inserter>
    static unsigned int simplifyPath(It it, It end, Inserter out)
    {
        const PathVertex* curr = &*it;
        *out = *it++;

        unsigned int appendedCount = 1;

        int directionX = 0;
        int directionY = 0;

        for (; it != end; ++it)
        {
            const PathVertex* next = &*it;
            int nextDirectionX = next->position.x - curr->position.x;
            int nextDirectionY = next->position.y - curr->position.y;

            // if both edges travel in the same direction,
            // replace the current vertex with next
            if (directionX == nextDirectionX && directionY == nextDirectionY)
            {
                *out = *next;
            }
            else // otherwise append it and remember the new direction
            {
                *++out = *next;
                directionX = nextDirectionX;
                directionY = nextDirectionY;
                appendedCount += 1;
            }

            curr = next;
        }

        return appendedCount;
    }

    std::vector<PathVertex> runSimplifyPath(const std::vector<PathVertex>& input);
}

#endif
