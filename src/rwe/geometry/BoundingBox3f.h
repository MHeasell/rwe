#pragma once

#include <rwe/math/Vector3f.h>

namespace rwe
{
    /** An axis-aligned bounding box in 3D space */
    struct BoundingBox3f
    {
        static BoundingBox3f fromMinMax(const Vector3f& min, const Vector3f& max);

        /** The position of the center of the box. */
        Vector3f center;

        /** The box's extents in all directions from the center. */
        Vector3f extents;

        BoundingBox3f(const Vector3f& center, const Vector3f& extents);

        float distanceSquared(const Vector3f& pos) const;
    };
}
