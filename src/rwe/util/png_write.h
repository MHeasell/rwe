#pragma once

#include <cstdint>
#include <fstream>
#include <ostream>
#include <png.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace rwe
{
    struct RgbPixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    class PngImage
    {
    public:
        uint32_t width;
        uint32_t height;
        std::vector<RgbPixel> pixels;

        PngImage(uint32_t width, uint32_t height)
            : width(width), height(height), pixels(width * height)
        {
        }

        RgbPixel& at(uint32_t x, uint32_t y)
        {
            return pixels[y * width + x];
        }

        void write(const std::string& filename) const
        {
            FILE* fp = fopen(filename.c_str(), "wb");
            if (!fp)
            {
                throw std::runtime_error("Failed to open file for writing: " + filename);
            }

            png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            if (!png)
            {
                fclose(fp);
                throw std::runtime_error("Failed to create PNG write struct");
            }

            png_infop info = png_create_info_struct(png);
            if (!info)
            {
                png_destroy_write_struct(&png, nullptr);
                fclose(fp);
                throw std::runtime_error("Failed to create PNG info struct");
            }

            if (setjmp(png_jmpbuf(png)))
            {
                png_destroy_write_struct(&png, &info);
                fclose(fp);
                throw std::runtime_error("Error during PNG write");
            }

            png_init_io(png, fp);
            writeRows(png, info);
            png_destroy_write_struct(&png, &info);
            fclose(fp);
        }

        void writeStream(std::ostream& out) const
        {
            png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            if (!png)
            {
                throw std::runtime_error("Failed to create PNG write struct");
            }

            png_infop info = png_create_info_struct(png);
            if (!info)
            {
                png_destroy_write_struct(&png, nullptr);
                throw std::runtime_error("Failed to create PNG info struct");
            }

            if (setjmp(png_jmpbuf(png)))
            {
                png_destroy_write_struct(&png, &info);
                throw std::runtime_error("Error during PNG write");
            }

            png_set_write_fn(
                png,
                &out,
                [](png_structp pngPtr, png_bytep data, png_size_t length) {
                    auto* stream = static_cast<std::ostream*>(png_get_io_ptr(pngPtr));
                    stream->write(reinterpret_cast<const char*>(data), length);
                },
                nullptr);

            writeRows(png, info);
            png_destroy_write_struct(&png, &info);
        }

    private:
        void writeRows(png_structp png, png_infop info) const
        {
            png_set_IHDR(
                png,
                info,
                width,
                height,
                8,
                PNG_COLOR_TYPE_RGB,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);

            png_write_info(png, info);

            for (uint32_t y = 0; y < height; ++y)
            {
                png_write_row(png, reinterpret_cast<png_bytep>(const_cast<RgbPixel*>(&pixels[y * width])));
            }

            png_write_end(png, nullptr);
        }
    };

    inline void loadPalette(const std::string& filename, RgbPixel* buffer)
    {
        std::ifstream in(filename, std::ios::binary);
        for (unsigned int i = 0; i < 256; ++i)
        {
            in.read(reinterpret_cast<char*>(&(buffer[i].r)), 1);
            in.read(reinterpret_cast<char*>(&(buffer[i].g)), 1);
            in.read(reinterpret_cast<char*>(&(buffer[i].b)), 1);
            in.seekg(1, std::ios::cur); // skip alpha
        }
    }

    inline void loadPalette(std::istream& in, RgbPixel* buffer)
    {
        for (unsigned int i = 0; i < 256; ++i)
        {
            in.read(reinterpret_cast<char*>(&(buffer[i].r)), 1);
            in.read(reinterpret_cast<char*>(&(buffer[i].g)), 1);
            in.read(reinterpret_cast<char*>(&(buffer[i].b)), 1);
            in.seekg(1, std::ios::cur); // skip alpha
        }
    }
}
