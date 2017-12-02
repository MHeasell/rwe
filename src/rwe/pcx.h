#ifndef RWE_PCX_H
#define RWE_PCX_H

#include <algorithm>
#include <cstdint>
#include <rwe/ColorPalette.h>
#include <stdexcept>
#include <vector>

namespace rwe
{
    class PcxException : public std::runtime_error
    {
    public:
        explicit PcxException(const char* message);
    };

#pragma pack(1)

    struct PcxWindow
    {
        uint16_t xMin;
        uint16_t yMin;
        uint16_t xMax;
        uint16_t yMax;
    };

    struct PcxHeader
    {
        uint8_t manufacturer;
        uint8_t version;

        uint8_t encoding;
        uint8_t bitsPerPixel;
        PcxWindow window;
        uint16_t horizontalDpi;
        uint16_t verticalDpi;
        uint8_t colorMap[48];
        uint8_t reserved;
        uint8_t numberOfPlanes;
        uint16_t bytesPerLine;
        uint16_t paletteInfo;
        uint16_t horizontalScreenSize;
        uint16_t verticalScreenSize;

        uint8_t filler[54];
    };

    struct PcxPaletteColor
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;

        Color toColor() const { return Color(red, green, blue); }
    };

#pragma pack()

    class PcxDecoder
    {
    private:
        const char* begin;
        const char* end;
        const PcxHeader* header;

    public:
        PcxDecoder(const char* begin, const char* end);

        unsigned int getWidth()
        {
            return (header->window.xMax - header->window.xMin) + 1u;
        }

        unsigned int getHeight()
        {
            return (header->window.yMax - header->window.yMin) + 1u;
        }

        std::vector<char> decodeImage()
        {
            auto ySize = getHeight();

            auto totalBytesPerRow = header->numberOfPlanes * header->bytesPerLine;
            auto totalBytes = totalBytesPerRow * ySize;

            std::vector<char> vec(totalBytes);
            auto buf = vec.data();

            auto it = begin + sizeof(PcxHeader);

            for (std::size_t y = 0; y < ySize; ++y)
            {
                auto rowBuf = buf + (y * totalBytesPerRow);

                auto bytesWritten = 0;
                while (bytesWritten < totalBytesPerRow)
                {
                    if (it == end)
                    {
                        throw PcxException("reached end of input before row could be finished");
                    }
                    auto byte = *(it++);
                    if ((byte & 0b11000000) == 0b11000000)
                    {
                        auto count = byte & 0b00111111;
                        if (it == end)
                        {
                            throw PcxException("malformed row");
                        }
                        auto data = *(it++);
                        std::fill_n(rowBuf + bytesWritten, count, data);
                        bytesWritten += count;
                    }
                    else
                    {
                        rowBuf[bytesWritten++] = byte;
                    }
                }
            }

            return vec;
        }

        std::vector<PcxPaletteColor> decodePalette()
        {
            auto it = end - ((sizeof(PcxPaletteColor) * 256) + 1);
            if (*(it++) != 12)
            {
                throw PcxException("invalid palette");
            }

            auto colorIt = reinterpret_cast<const PcxPaletteColor*>(it);

            std::vector<PcxPaletteColor> colors(256);

            std::copy(colorIt, colorIt + 256, colors.data());

            return colors;
        }
    };
}

#endif
