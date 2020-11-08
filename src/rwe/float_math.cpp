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

    float rweLerp(float a, float b, float t)
    {
        return (1.0f - t) * a + t * b;
    }
}
