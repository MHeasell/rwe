#ifndef RWE_GEOMETRY_TRANGLE3F_H
#define RWE_GEOMETRY_TRANGLE3F_H

#include <boost/optional.hpp>
#include <rwe/math/Vector3f.h>
#include <rwe/geometry/Ray3f.h>
#include <rwe/geometry/Plane3f.h>

namespace rwe
{
    struct Triangle3f
    {
        Vector3f a;
        Vector3f b;
        Vector3f c;

        Triangle3f() = default;

        Triangle3f(const Vector3f& a, const Vector3f& b, const Vector3f& c);

        /**
         * Returns the distance along the ray
         * at which it intersects with this triangle.
         * The distance is defined in terms of the ray's
         * direction vector.
         *
         * If the ray and the triangle never intersect,
         * this will return a result indicating that they did not intersect.
         */
        boost::optional<float> intersect(const Ray3f& ray) const;

        /**
         * Returns the point at which the given line intersects this triangle.
         * If the line and the triangle do not intersect,
         * returns a result indicating this.
         *
         * The intersection test is performed using scalar triple product.
         * If another triangle shares an edge with this one,
         * the test guarantees that the line intersects
         * either with this triangle or the other triangle,
         * provided that the edge goes in the same direction
         * in both triangles.
         * (That is, any of A->B, B->C, C->A)
         *
         * @param p The point at which the line starts
         * @param q The point at which the line ends
         */
        boost::optional<Vector3f> intersectLine(const Vector3f& p, const Vector3f& q) const;

        boost::optional<Vector3f> intersectLine(const Line3f& line) const;

        /**
         * Converts the input world-space coordinates
         * which lie on the triangle into barycentric coordinates.
         */
        Vector3f toBarycentric(const Vector3f& p) const;

        /**
         * Converts the input barycentric coordinates
         * into world-space coordinates.
         */
        Vector3f toCartesian(const Vector3f& p) const;

        /**
         * Returns the plane on which this triangle lies.
         */
        Plane3f toPlane() const;
    };
}

#endif
