#include <cmath>
#include "OctileDistance.h"

namespace rwe
{
    static constexpr float DiagonalDistance = std::sqrt(2.0f);

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
