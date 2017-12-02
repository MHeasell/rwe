#ifndef RWE_COLORPALETTE_H
#define RWE_COLORPALETTE_H

#include <boost/optional.hpp>
#include <cstdint>
#include <vector>

namespace rwe
{
#pragma pack(1)
    struct Color
    {
        static const Color Black;
        static const Color Transparent;

        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        Color() = default;
        Color(unsigned char r, unsigned char g, unsigned char b) noexcept;
        Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) noexcept;
    };
#pragma pack()

    using ColorPalette = std::vector<Color>;

    void readColorPalette(const char* input, Color* output);

    boost::optional<ColorPalette> readPalette(std::vector<char>& vector);
}

#endif
