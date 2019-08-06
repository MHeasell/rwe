#pragma once

#include <rwe/math/Vector3x.h>

namespace rwe
{
    /** An axis-aligned bounding box in 3D space */
    template <typename Val>
    struct BoundingBox3x
    {
        static BoundingBox3x fromMinMax(const Vector3x<Val>& min, const Vector3x<Val>& max)
        {
            auto center = (min + max) / Val(2);
            auto extents = (max - min) / Val(2);
            return BoundingBox3x(center, extents);
        }

        /** The position of the center of the box. */
        Vector3x<Val> center;

        /** The box's extents in all directions from the center. */
        Vector3x<Val> extents;

        BoundingBox3x(const Vector3x<Val>& center, const Vector3x<Val>& extents)
            : center(center),
              extents(extents)
        {
        }

        Val distanceSquared(const Vector3x<Val>& pos) const
        {
            auto toCenter = center - pos;
            auto dX = std::max(Val(0), abs(toCenter.x) - extents.x);
            auto dY = std::max(Val(0), abs(toCenter.y) - extents.y);
            auto dZ = std::max(Val(0), abs(toCenter.z) - extents.z);
            return (dX * dX) + (dY * dY) + (dZ * dZ);
        }
    };
}
