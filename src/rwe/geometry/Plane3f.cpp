#include "Plane3f.h"

namespace rwe
{
    float intersectOrInfinity(const Plane3f& plane, const Ray3f& ray)
    {
        auto result = plane.intersect(ray);
        if (result)
        {
            return *result;
        }

        return plane.isInFront(ray.origin)
            ? ::std::numeric_limits<float>::infinity()
            : -::std::numeric_limits<float>::infinity();
    }
}
