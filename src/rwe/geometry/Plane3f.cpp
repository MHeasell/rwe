#include "Plane3f.h"

namespace rwe
{
    Plane3f::Plane3f(const Vector3f& point, const Vector3f& normal) : point(point), normal(normal) {}

    Plane3f Plane3f::fromPoints(const Vector3f& a, const Vector3f& b, const Vector3f& c)
    {
        auto position = a;

        auto v1 = b - a;
        auto v2 = c - a;

        auto normal = v1.cross(v2);

        return Plane3f(position, normal);
    }

    std::optional<float> Plane3f::intersect(const Ray3f& ray) const
    {
        float a = (point - ray.origin).dot(normal);
        float b = ray.direction.dot(normal);
        if (b == 0.0f)
        {
            return std::nullopt;
        }

        return a / b;
    }

    bool Plane3f::isInFront(const Vector3f& p) const
    {
        return (p - point).dot(normal) > 0;
    }

    float Plane3f::intersectOrInfinity(const Ray3f& ray) const
    {
        auto result = intersect(ray);
        if (result)
        {
            return *result;
        }

        return isInFront(ray.origin)
            ? ::std::numeric_limits<float>::infinity()
            : -::std::numeric_limits<float>::infinity();
    }

    std::optional<Vector3f>
    Plane3f::intersectLine(const Vector3f& startPoint, const Vector3f& endPoint)
    {
        Ray3f ray(startPoint, endPoint - startPoint);
        auto result = intersect(ray);
        if (result && *result >= 0.0f && *result <= 1.0f)
        {
            return ray.pointAt(*result);
        }

        return std::nullopt;
    }
}
