#include "RadiansAngle.h"
#include "util.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    RadiansAngle::RadiansAngle(float value) : OpaqueId(value)
    {
        assert(value >= -Pif);
        assert(value < Pif);
    }

    RadiansAngle RadiansAngle::operator-(RadiansAngle rhs)
    {
        return RadiansAngle(wrap(-Pif, Pif, value - rhs.value));
    }
}
