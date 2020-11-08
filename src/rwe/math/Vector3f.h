#pragma once

#include <optional>
#include <ostream>
#include <rwe/float_math.h>
#include <rwe/math/Vector3x.h>

namespace rwe
{
    using Vector3f = Vector3x<float>;

    float angleTo(const Vector3f& lhs, const Vector3f& rhs, const Vector3f& normal);

    Vector3f lerp(const Vector3f& a, const Vector3f& b, float t);
}
