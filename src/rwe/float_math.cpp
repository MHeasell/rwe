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
}
