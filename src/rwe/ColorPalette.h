#ifndef RWE_COLORPALETTE_H
#define RWE_COLORPALETTE_H

#include <cstdint>

namespace rwe
{
    struct Color
    {
        unsigned char r;
        unsigned char g;
        unsigned char b;

        Color() = default;
        Color(unsigned char r, unsigned char g, unsigned char b);
    };

    void readColorPalette(const char* input, Color* output);
}

#endif
