#pragma once

#include <optional>
#include <ostream>
#include <rwe/math/Vector3x.h>

namespace rwe
{
    using Vector3f = Vector3x<float>;

    float angleTo(const Vector3f& lhs, const Vector3f& rhs, const Vector3f& normal);
}
