#ifndef RWE_GEOMETRY_PLANE3F_H
#define RWE_GEOMETRY_PLANE3F_H

#include <boost/optional.hpp>
#include <rwe/geometry/Ray3f.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    /** An infinite plane in 3D space. */
    struct Plane3f
    {
        /**
         * Creates a plane from 3 points that lie on it.
         * The plane is constructed such that the points
         * are in anticlockwise order when the normal
         * is pointing at the viewer.
         */
        static Plane3f fromPoints(const Vector3f& a, const Vector3f& b, const Vector3f& c);

        /** A point on the plane. */
        Vector3f point;

        /** The normal of the plane. */
        Vector3f normal;

        Plane3f(const Vector3f& point, const Vector3f& normal);

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
        boost::optional<float> intersect(const Ray3f& ray) const;

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
        float intersectOrInfinity(const Ray3f& ray) const;

        boost::optional<Vector3f> intersectLine(const Vector3f& startPoint, const Vector3f& endPoint);

        /**
         * Returns true if the given point is in front of the plane.
         * The point is in front of the plane if it is on the side of the plane
         * that the normal is pointing to.
         * Points that lie on the plane are not considered to be in front of it.
         */
        bool isInFront(const Vector3f& p) const;
    };
}

#endif
