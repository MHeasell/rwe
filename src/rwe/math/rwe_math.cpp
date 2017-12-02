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
        return (a * b) >= 0.0f;
    }

    Vector2f convertScreenToClipSpace(unsigned int screenWidth, unsigned int screenHeight, Point p)
    {
        auto halfScreenWidth = static_cast<float>(screenWidth) / 2.0f;
        auto halfScreenHeight = static_cast<float>(screenHeight) / 2.0f;

        auto x = (static_cast<float>(p.x) / halfScreenWidth) - 1.0f;
        auto y = (static_cast<float>(screenHeight - p.y) / halfScreenHeight) - 1.0f;
        return Vector2f(x, y);
    }

    float wrap(float min, float max, float value)
    {
        assert(min <= max);

        auto range = max - min;
        auto offsetVal = value - min;
        auto modVal = std::fmod((std::fmod(offsetVal, range) + range), range);
        return modVal + min;
    }
}
