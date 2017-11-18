#ifndef RWE_GRIDREGION_H
#define RWE_GRIDREGION_H

namespace rwe
{
    struct GridRegion
    {
        unsigned int x;
        unsigned int y;
        unsigned int width;
        unsigned int height;

        GridRegion() = default;
        GridRegion(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    };
}

#endif
