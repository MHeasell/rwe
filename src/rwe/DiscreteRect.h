#ifndef RWE_DISCRETERECT_H
#define RWE_DISCRETERECT_H

namespace rwe
{
    struct DiscreteRect
    {
        int x;
        int y;
        unsigned int width;
        unsigned int height;

        DiscreteRect() = default;
        DiscreteRect(int x, int y, unsigned int width, unsigned int height) : x(x), y(y), width(width), height(height)
        {
        }
    };
}

#endif
