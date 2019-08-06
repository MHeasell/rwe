#pragma once

#include <optional>
#include <rwe/geometry/Ray3x.h>
#include <rwe/math/Vector3x.h>

namespace rwe
{
    /** An infinite plane in 3D space. */
    template <typename Val>
    struct Plane3x
    {
        /**
         * Creates a plane from 3 points that lie on it.
         * The plane is constructed such that the points
         * are in anticlockwise order when the normal
         * is pointing at the viewer.
         */
        static Plane3x fromPoints(const Vector3x<Val>& a, const Vector3x<Val>& b, const Vector3x<Val>& c)
        {
            auto position = a;

            auto v1 = b - a;
            auto v2 = c - a;

            auto normal = v1.cross(v2);

            return Plane3x(position, normal);
        }

        /** A point on the plane. */
        Vector3x<Val> point;

        /** The normal of the plane. */
        Vector3x<Val> normal;

        Plane3x(const Vector3x<Val>& point, const Vector3x<Val>& normal) : point(point), normal(normal) {}

        /**
         * Returns the distance along the ray
         * at which it intersects with this plane.
         * The distance is defined in terms of the ray's
         * direction vector.
         *
         * If the ray and the plane never intersect,
         * i.e. they are parallel, this will return
         * a result indicating that they did not intersect.
         */
        std::optional<Val> intersect(const Ray3x<Val>& ray) const
        {
            auto a = (point - ray.origin).dot(normal);
            auto b = ray.direction.dot(normal);
            if (b == Val(0))
            {
                return std::nullopt;
            }

            return a / b;
        }

        std::optional<Vector3x<Val>> intersectLine(const Vector3x<Val>& startPoint, const Vector3x<Val>& endPoint)
        {
            Ray3x<Val> ray(startPoint, endPoint - startPoint);
            auto result = intersect(ray);
            if (result && *result >= 0 && *result <= 1)
            {
                return ray.pointAt(*result);
            }

            return std::nullopt;
        }

        /**
         * Returns true if the given point is in front of the plane.
         * The point is in front of the plane if it is on the side of the plane
         * that the normal is pointing to.
         * Points that lie on the plane are not considered to be in front of it.
         */
        bool isInFront(const Vector3x<Val>& p) const
        {
            return (p - point).dot(normal) > 0;
        }
    };
}
