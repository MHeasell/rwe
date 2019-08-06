#pragma once

#include <optional>
#include <rwe/geometry/Line3x.h>
#include <rwe/geometry/Plane3x.h>
#include <rwe/geometry/Ray3x.h>
#include <rwe/math/Vector3x.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    template <typename Val>
    struct Triangle3x
    {
        Vector3x<Val> a;
        Vector3x<Val> b;
        Vector3x<Val> c;

        Triangle3x() = default;

        Triangle3x(const Vector3x<Val>& a, const Vector3x<Val>& b, const Vector3x<Val>& c) : a(a), b(b), c(c)
        {
        }

        /**
         * Returns the distance along the ray
         * at which it intersects with this triangle.
         * The distance is defined in terms of the ray's
         * direction vector.
         *
         * If the ray and the triangle never intersect,
         * this will return a result indicating that they did not intersect.
         */
        std::optional<Val> intersect(const Ray3x<Val>& ray) const
        {
            auto intersect = toPlane().intersect(ray);
            if (!intersect)
            {
                return std::nullopt;
            }

            Vector3x<Val> p = toBarycentric(ray.pointAt(*intersect));
            if (p.x < 0 || p.y < 0 || p.z < 0)
            {
                return std::nullopt;
            }

            return *intersect;
        }

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
        std::optional<Vector3x<Val>> intersectLine(const Vector3x<Val>& p, const Vector3x<Val>& q) const
        {
            auto pq = q - p;
            auto pa = a - p;
            auto pb = b - p;
            auto pc = c - p;

            auto u = scalarTriple(pq, pc, pb);
            auto v = scalarTriple(pq, pa, pc);
            auto w = scalarTriple(pq, pb, pa);
            if (!sameSign(u, v, w))
            {
                return std::nullopt;
            }

            auto denominator = u + v + w;
            u /= denominator;
            v /= denominator;
            w /= denominator;

            return toCartesian(Vector3x<Val>(u, v, w));
        }

        std::optional<Vector3x<Val>> intersectLine(const Line3x<Val>& line) const
        {
            return intersectLine(line.start, line.end);
        }

        /**
         * Converts the input world-space coordinates
         * which lie on the triangle into barycentric coordinates.
         */
        Vector3x<Val> toBarycentric(const Vector3x<Val>& p) const
        {
            auto v0 = b - a;
            auto v1 = c - a;
            auto v2 = p - a;

            float v = ((v1.dot(v1) * v2.dot(v0)) - (v1.dot(v0) * v2.dot(v1)))
                / ((v0.dot(v0) * v1.dot(v1)) - (v0.dot(v1) * v1.dot(v0)));

            float w = ((v0.dot(v0) * v2.dot(v1)) - (v0.dot(v1) * v2.dot(v0)))
                / ((v0.dot(v0) * v1.dot(v1)) - (v0.dot(v1) * v1.dot(v0)));

            float u = 1.0f - v - w;

            return Vector3x<Val>(u, v, w);
        }

        /**
         * Converts the input barycentric coordinates
         * into world-space coordinates.
         */
        Vector3x<Val> toCartesian(const Vector3x<Val>& p) const
        {
            return (a * p.x) + (b * p.y) + (c * p.z);
        }

        /**
         * Returns the plane on which this triangle lies.
         */
        Plane3x<Val> toPlane() const
        {
            return Plane3x<Val>::fromPoints(a, b, c);
        }
    };

    using Triangle3f = Triangle3x<float>;
}
