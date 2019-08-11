#pragma once

#include <rwe/math/Vector2f.h>

namespace rwe
{
    struct Circle2f
    {
        float radius;
        Vector2f position;

        Circle2f() = default;
        Circle2f(float radius, const Vector2f& position);

        bool contains(const Vector2f& point) const;
    };
}
