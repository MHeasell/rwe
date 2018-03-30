#include "RadiansAngle.h"
#include <rwe/math/rwe_math.h>
#include <rwe/util.h>

namespace rwe
{
    RadiansAngle RadiansAngle::fromUnwrappedAngle(float value)
    {
        return RadiansAngle(wrap(-Pif, Pif, value));
    }

    RadiansAngle::RadiansAngle(float value) : OpaqueId(value)
    {
        assert(value >= -Pif);
        assert(value < Pif);
    }

    RadiansAngle RadiansAngle::operator-(RadiansAngle rhs)
    {
        return fromUnwrappedAngle(value - rhs.value);
    }
}
