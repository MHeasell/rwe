#include "float_math.h"

#include <cmath>

namespace rwe
{
    float rweAbs(float v)
    {
        return std::abs(v);
    }

    float rweSqrt(float f)
    {
        return std::sqrt(f);
    }

    float rweCos(float v)
    {
        return std::cos(v);
    }

    float rweSin(float v)
    {
        return std::sin(v);
    }

    float rweAcos(float v)
    {
        return std::acos(v);
    }

    float rweAtan2(float a, float b)
    {
        return std::atan2(a, b);
    }
}
