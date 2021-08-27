#pragma once

#include <rwe/pathfinding/OctileDistance.h>

namespace rwe
{
    struct PathCost
    {
        OctileDistance distance;
        int turnCount{0};

        PathCost() = default;
        PathCost(const OctileDistance& distance, int turnCount);

        bool operator==(const PathCost& rhs) const;

        bool operator!=(const PathCost& rhs) const;

        bool operator<(const PathCost& rhs) const;

        bool operator>(const PathCost& rhs) const;

        bool operator<=(const PathCost& rhs) const;

        bool operator>=(const PathCost& rhs) const;

        PathCost operator+(const PathCost& rhs) const;
    };
}
