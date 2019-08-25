#include "BoundingBox3f.h"
#include <cmath>

namespace rwe
{
    BoundingBox3f BoundingBox3f::fromMinMax(const Vector3f& min, const Vector3f& max)
    {
        auto center = (min + max) / 2.0f;
        auto extents = (max - min) / 2.0f;
        return BoundingBox3f(center, extents);
    }

    BoundingBox3f::BoundingBox3f(const Vector3f& center, const Vector3f& extents)
        : center(center),
          extents(extents)
    {
    }

    float BoundingBox3f::distanceSquared(const Vector3f& pos) const
    {
        auto toCenter = center - pos;
        auto dX = std::fmax(0.0f, std::abs(toCenter.x) - extents.x);
        auto dY = std::fmax(0.0f, std::abs(toCenter.y) - extents.y);
        auto dZ = std::fmax(0.0f, std::abs(toCenter.z) - extents.z);
        return (dX * dX) + (dY * dY) + (dZ * dZ);
    }
}
