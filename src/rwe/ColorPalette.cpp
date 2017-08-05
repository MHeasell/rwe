#include "ColorPalette.h"

namespace rwe
{
    Color::Color(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}

    void readColorPalette(const char* input, Color* output)
    {
        for (std::size_t i = 0; i < 256; ++i)
        {
            output[i].r = static_cast<unsigned char>(input[(3 * i)]);
            output[i].g = static_cast<unsigned char>(input[(3 * i) + 1]);
            output[i].b = static_cast<unsigned char>(input[(3 * i) + 2]);
        }
    }
}
