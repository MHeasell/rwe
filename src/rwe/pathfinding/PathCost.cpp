#include "PathCost.h"

namespace rwe
{
    PathCost::PathCost(const OctileDistance& distance, int turnCount)
        : distance(distance), turnCount(turnCount)
    {
    }

    bool PathCost::operator==(const PathCost& rhs) const
    {
        return distance == rhs.distance && turnCount == rhs.turnCount;
    }

    bool PathCost::operator!=(const PathCost& rhs) const
    {
        return !(rhs == *this);
    }

    bool PathCost::operator<(const PathCost& rhs) const
    {
        if (distance < rhs.distance)
        {
            return true;
        }

        if (rhs.distance < distance)
        {
            return false;
        }

        return turnCount < rhs.turnCount;
    }

    bool PathCost::operator>(const PathCost& rhs) const
    {
        return rhs < *this;
    }

    bool PathCost::operator<=(const PathCost& rhs) const
    {
        return !(rhs < *this);
    }

    bool PathCost::operator>=(const PathCost& rhs) const
    {
        return !(*this < rhs);
    }

    PathCost PathCost::operator+(const PathCost& rhs) const
    {
        return PathCost(distance + rhs.distance, turnCount + rhs.turnCount);
    }
}
