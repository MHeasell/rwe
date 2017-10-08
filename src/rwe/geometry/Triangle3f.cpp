#include "Triangle3f.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    Triangle3f::Triangle3f(const Vector3f& a, const Vector3f& b, const Vector3f& c) : a(a), b(b), c(c)
    {
    }

    boost::optional<float> Triangle3f::intersect(const Ray3f& ray) const
    {
        auto intersect = toPlane().intersect(ray);
        if (!intersect)
        {
            return boost::none;
        }

        Vector3f p = toBarycentric(ray.pointAt(*intersect));
        if (p.x < 0 || p.y < 0 || p.z < 0)
        {
            return boost::none;
        }

        return *intersect;
    }

    boost::optional<Vector3f> Triangle3f::intersectLine(const Vector3f& p, const Vector3f& q) const
    {
        auto pq = q - p;
        auto pa = a - p;
        auto pb = b - p;
        auto pc = c - p;

        float u = scalarTriple(pq, pc, pb);
        float v = scalarTriple(pq, pa, pc);
        if (!sameSign(u, v))
        {
            return boost::none;
        }

        float w = scalarTriple(pq, pb, pa);
        if (!sameSign(v, w))
        {
            return boost::none;
        }

        float denominator = u + v + w;
        u /= denominator;
        v /= denominator;
        w /= denominator;

        return toCartesian(Vector3f(u, v, w));
    }

    Plane3f Triangle3f::toPlane() const
    {
        return Plane3f::fromPoints(a, b, c);
    }

    Vector3f Triangle3f::toBarycentric(const Vector3f& p) const
    {
        auto v0 = b - a;
        auto v1 = c - a;
        auto v2 = p - a;

        float v = ((v1.dot(v1) * v2.dot(v0)) - (v1.dot(v0) * v2.dot(v1)))
            / ((v0.dot(v0) * v1.dot(v1)) - (v0.dot(v1) * v1.dot(v0)));

        float w = ((v0.dot(v0) * v2.dot(v1)) - (v0.dot(v1) * v2.dot(v0)))
            / ((v0.dot(v0) * v1.dot(v1)) - (v0.dot(v1) * v1.dot(v0)));

        float u = 1.0f - v - w;

        return Vector3f(u, v, w);
    }

    Vector3f Triangle3f::toCartesian(const Vector3f& p) const
    {
        return (a * p.x) + (b * p.y) + (c * p.z);
    }

    boost::optional<Vector3f> Triangle3f::intersectLine(const Line3f& line) const
    {
        return intersectLine(line.start, line.end);
    }
}
