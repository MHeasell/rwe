#ifndef RWE_GEOMETRY_BOUNDINGBOX3F_H
#define RWE_GEOMETRY_BOUNDINGBOX3F_H

#include <boost/optional.hpp>
#include <rwe/geometry/Ray3f.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    /** An axis-aligned bounding box in 3D space */
    struct BoundingBox3f
    {
        struct RayIntersect
        {
            float enter;
            float exit;
            RayIntersect(float enter, float exit) : enter(enter), exit(exit) {}
        };

        /** The position of the center of the box. */
        Vector3f center;

        /** The box's extents in all directions from the center. */
        Vector3f extents;

        BoundingBox3f(const Vector3f& center, const Vector3f& extents);

        /**
         * Computes the intersection between the given ray and the bounding box.
         * If the ray intersects, returns a result containing the distances
         * at which the ray enters and exits the bounding box.
         * Otherwise, returns a result indicating that the ray missed.
         */
        boost::optional<RayIntersect> intersect(const Ray3f& ray) const;
    };
}

#endif
