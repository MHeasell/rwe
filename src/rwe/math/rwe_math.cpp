#include "rwe_math.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <stdexcept>


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
        return (a >= 0.0f && b >= 0.0f) || (a <= 0.0f && b <= 0.0f);
    }

    float wrap(float min, float max, float value)
    {
        assert(min < max);
        if (value >= min && value < max)
        {
            return value;
        }
        return wrap(value - min, max - min) + min;
    }

    float wrap(float value, float max)
    {
        assert(max > 0.0f);
        return std::fmod(std::fmod(value, max) + max, max);
    }

    int wrap(int min, int max, int value)
    {
        assert(min < max);
        return wrap(value - min, max - min) + min;
    }

    int wrap(int value, int max)
    {
        assert(max > 0);
        return ((value % max) + max) % max;
    }

    unsigned int roundUpToPowerOfTwo(unsigned int v)
    {
        // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;

        return v;
    }

    bool almostEquals(float a, float b, float delta)
    {
        return (std::abs(b - a) < delta);
    }

    float toRadians(float v)
    {
        return v * (Pif / 180.0f);
    }

    float rweLerp(float a, float b, float t)
    {
        return (1.0f - t) * a + t * b;
    }

    float angleLerp(float a, float b, float t)
    {
        if (b - a >= Pif)
        {
            return rweLerp(a + (2.0f * Pif), b, t);
        }
        if (b - a < -Pif)
        {
            return rweLerp(a, b + (2.0f * Pif), t);
        }
        return rweLerp(a, b, t);
    }
}
