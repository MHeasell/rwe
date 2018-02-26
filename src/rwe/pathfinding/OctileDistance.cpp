#include "OctileDistance.h"
#include <cmath>

namespace rwe
{
    static const float DiagonalDistance = std::sqrt(2.0f);

    OctileDistance OctileDistance::fromXAndY(unsigned int x, unsigned int y)
    {
        if (x > y)
        {
            return OctileDistance(x - y, y);
        }
        else
        {
            return OctileDistance(y - x, x);
        }
    }

    OctileDistance::OctileDistance(unsigned int straight, unsigned int diagonal) : straight(straight), diagonal(diagonal)
    {
    }

    bool OctileDistance::operator==(const OctileDistance& rhs) const
    {
        return straight == rhs.straight && diagonal == rhs.diagonal;
    }

    bool OctileDistance::operator!=(const OctileDistance& rhs) const
    {
        return !(rhs == *this);
    }

    bool OctileDistance::operator<(const OctileDistance& rhs) const
    {
        return asFloat() < rhs.asFloat();
    }

    bool OctileDistance::operator>(const OctileDistance& rhs) const
    {
        return asFloat() > rhs.asFloat();
    }

    bool OctileDistance::operator<=(const OctileDistance& rhs) const
    {
        return asFloat() <= rhs.asFloat();
    }

    bool OctileDistance::operator>=(const OctileDistance& rhs) const
    {
        return asFloat() >= rhs.asFloat();
    }

    OctileDistance OctileDistance::operator+(const OctileDistance& rhs) const
    {
        return OctileDistance(straight + rhs.straight, diagonal + rhs.diagonal);
    }

    float OctileDistance::asFloat() const
    {
        return straight + (diagonal * DiagonalDistance);
    }
}
