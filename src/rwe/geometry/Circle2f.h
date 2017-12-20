#ifndef RWE_CIRCLE2F_H
#define RWE_CIRCLE2F_H

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

#endif
