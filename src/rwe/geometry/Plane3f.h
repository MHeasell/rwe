#pragma once

#include <rwe/geometry/Plane3x.h>
#include <rwe/geometry/Ray3f.h>

namespace rwe
{
    using Plane3f = Plane3x<float>;

    /**
     * Returns the distance along the ray
     * at which it intersects with this plane.
     * The distance is defined in terms of the ray's
     * direction vector.
     *
     * If the ray does not intersect, i.e. it is parallel,
     * if the ray is in front of the plane we return infinity,
     * otherwise we return negative infinity.
     */
    float intersectOrInfinity(const Plane3f& plane, const Ray3f& ray);
}
