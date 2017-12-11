#ifndef RWE_PATHFINDING_UTILS_H
#define RWE_PATHFINDING_UTILS_H

#include <rwe/DiscreteRect.h>

namespace rwe
{
    /**
     * This method assumes that the input path has a minimum of one element.
     * If this is not the case, the behaviour is undefined.
     */
    template <typename It, typename Inserter>
    static unsigned int simplifyPath(It it, It end, Inserter out)
    {
        const DiscreteRect* curr = &*it;
        *out = *it++;

        unsigned int appendedCount = 1;

        int directionX = 0;
        int directionY = 0;

        for (; it != end; ++it)
        {
            const DiscreteRect* next = &*it;
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

    std::vector<DiscreteRect> runSimplifyPath(const std::vector<DiscreteRect>& input)
    {
        std::vector<DiscreteRect> output(input.size());
        auto count = simplifyPath(input.begin(), input.end(), output.begin());
        output.resize(count);
        return output;
    }
}

#endif
