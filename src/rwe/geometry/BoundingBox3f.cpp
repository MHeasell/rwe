#include "BoundingBox3f.h"
#include <cmath>
#include <rwe/geometry/Plane3f.h>

namespace rwe
{
    BoundingBox3f::BoundingBox3f(const Vector3f& center, const Vector3f& extents)
        : center(center),
          extents(extents)
    {
    }

    boost::optional<BoundingBox3f::RayIntersect> BoundingBox3f::intersect(const Ray3f& ray) const
    {
        Plane3f bottomPlane(center + Vector3f(0.0f, 0.0f, -extents.z), Vector3f(0.0f, 0.0f, -1.0f));
        Plane3f topPlane(center + Vector3f(0.0f, 0.0f, extents.z), Vector3f(0.0f, 0.0f, -1.0f));

        float bottomIntersect = bottomPlane.intersectOrInfinity(ray);
        float topIntersect = topPlane.intersectOrInfinity(ray);

        float zEnter;
        float zExit;
        if (topIntersect > bottomIntersect)
        {
            zEnter = bottomIntersect;
            zExit = topIntersect;
        }
        else
        {
            zEnter = topIntersect;
            zExit = bottomIntersect;
        }

        Plane3f leftPlane(center + Vector3f(-extents.x, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f));
        Plane3f rightPlane(center + Vector3f(extents.x, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f));

        float leftIntersect = leftPlane.intersectOrInfinity(ray);
        float rightIntersect = rightPlane.intersectOrInfinity(ray);

        float xEnter;
        float xExit;
        if (rightIntersect > leftIntersect)
        {
            xEnter = leftIntersect;
            xExit = rightIntersect;
        }
        else
        {
            xEnter = rightIntersect;
            xExit = leftIntersect;
        }

        Plane3f nearPlane(center + Vector3f(0.0f, -extents.y, 0.0f), Vector3f(0.0f, -1.0f, 0.0f));
        Plane3f farPlane(center + Vector3f(0.0f, extents.y, 0.0f), Vector3f(0.0f, -1.0f, 0.0f));

        float nearIntersect = nearPlane.intersectOrInfinity(ray);
        float farIntersect = farPlane.intersectOrInfinity(ray);

        float yEnter;
        float yExit;
        if (farIntersect > nearIntersect)
        {
            yEnter = nearIntersect;
            yExit = farIntersect;
        }
        else
        {
            yEnter = farIntersect;
            yExit = nearIntersect;
        }

        float enter = std::fmax(std::fmax(xEnter, yEnter), zEnter);
        float exit = std::fmin(std::fmin(xExit, yExit), zExit);

        if (std::isfinite(enter) && std::isfinite(exit) && enter <= exit)
        {
            return RayIntersect(enter, exit);
        }

        // return a miss
        return boost::none;
    }
}
