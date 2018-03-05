#ifndef RWE_PATHCOST_H
#define RWE_PATHCOST_H

#include <rwe/pathfinding/OctileDistance.h>

namespace rwe
{
    struct PathCost
    {
        OctileDistance distance;
        unsigned int turnCount{0};

        PathCost() = default;
        PathCost(const OctileDistance& distance, unsigned int turnCount);

        bool operator==(const PathCost& rhs) const;

        bool operator!=(const PathCost& rhs) const;

        bool operator<(const PathCost& rhs) const;

        bool operator>(const PathCost& rhs) const;

        bool operator<=(const PathCost& rhs) const;

        bool operator>=(const PathCost& rhs) const;

        PathCost operator+(const PathCost& rhs) const;
    };
}

#endif
