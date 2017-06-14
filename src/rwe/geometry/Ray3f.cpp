#include "Ray3f.h"

namespace rwe
{
    Ray3f::Ray3f(const Vector3f& position, const Vector3f& direction) : origin(position), direction(direction) {}

    Vector3f Ray3f::pointAt(const float t) const
    {
        return origin + (direction * t);
    }

    bool Ray3f::isLessFar(const Vector3f& a, const Vector3f& b) const
    {
        return (a - origin).dot(direction) < (b - origin).dot(direction);
    }
}
