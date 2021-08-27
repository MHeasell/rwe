#pragma once

#include <rwe/grid/DiscreteRect.h>
#include <rwe/grid/EightWayDirection.h>
#include <rwe/grid/Point.h>
#include <rwe/pathfinding/OctileDistance.h>

namespace rwe
{
    /**
     * This method assumes that the input path has a minimum of one element.
     * If this is not the case, the behaviour is undefined.
     */
    template <typename It, typename Inserter>
    static int simplifyPath(It it, It end, Inserter out)
    {
        const Point* curr = &*it;
        *out = *it++;

        int appendedCount = 1;

        int directionX = 0;
        int directionY = 0;

        for (; it != end; ++it)
        {
            const Point* next = &*it;
            int nextDirectionX = next->x - curr->x;
            int nextDirectionY = next->y - curr->y;

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

    std::vector<Point> runSimplifyPath(const std::vector<Point>& input);

    OctileDistance octileDistance(const Point& a, const Point& b);
}
