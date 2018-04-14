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

    int wrap(int min, int max, int value)
    {
        assert(min <= max);

        auto range = max - min;
        auto offsetVal = value - min;
        auto modVal = ((offsetVal % range) + range) % range;
        return modVal + min;
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

    float distanceToRange(float min, float max, float pos)
    {
        assert(min <= max);
        if (pos < min)
        {
            return min - pos;
        }
        if (pos > max)
        {
            return pos - max;
        }
        return 0.0f;
    }

    float distanceSquaredToRange(float min, float max, float pos)
    {
        auto d = distanceToRange(min, max, pos);
        return d * d;
    }
}
