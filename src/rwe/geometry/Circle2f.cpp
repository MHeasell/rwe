#include "Circle2f.h"

namespace rwe
{
    Circle2f::Circle2f(float radius, const Vector2f& position) : radius(radius), position(position)
    {
    }

    bool Circle2f::contains(const Vector2f& point) const
    {
        return position.distanceSquared(point) <= (radius * radius);
    }
}
