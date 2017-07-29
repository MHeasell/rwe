#include "rwe_math.h"
#include <cstdlib>
#include <stdexcept>
#include <cmath>


namespace rwe
{
    float snapToInterval(float value, float interval)
    {
        return std::round(value / interval) * interval;
    }

    float truncateToInterval(float value, float interval)
    {
        return std::trunc(value / interval) * interval;
    }

    bool sameSign(float a, float b)
    {
        return (a * b) >= 0.0f;
    }
}
