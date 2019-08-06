#pragma once

#include <rwe/math/Vector2x.h>

namespace rwe
{
    template <typename Val>
    struct Circle2x
    {
        Val radius;
        Vector2x<Val> position;

        Circle2x() = default;
        Circle2x(Val radius, const Vector2x<Val>& position) : radius(radius), position(position)
        {
        }

        bool contains(const Vector2x<Val>& point) const
        {
            return position.distanceSquared(point) <= (radius * radius);
        }
    };
}
