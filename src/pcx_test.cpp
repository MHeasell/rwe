#include <iostream>
#include <png++/png.hpp>
#include <rwe/io/pcx/pcx.h>
#include <string>

class PaletteReadingException : public std::runtime_error
{
public:
    explicit PaletteReadingException(const char* message) : runtime_error(message) {}
};

int convert(const std::string& inFile, const std::string& outFile)
{
    std::ifstream in(inFile, std::ios::binary | std::ios::ate);

    // not guaranteed to be size in bytes, but everyone seems to abuse this
    auto size = static_cast<std::size_t>(in.tellg());
    in.seekg(0);

    std::vector<char> data(size);
    in.read(data.data(), size);

    rwe::PcxDecoder<std::vector<char>::const_iterator> decoder(data.begin(), data.end());

    auto decodedData = decoder.decodeImage();
    auto palette = decoder.decodePalette();

    auto width = decoder.getWidth();
    auto height = decoder.getHeight();

    png::image<png::rgb_pixel> image(width, height);
    for (png::uint_32 y = 0; y < height; ++y)
    {
        for (png::uint_32 x = 0; x < width; ++x)
        {
            auto b = static_cast<unsigned char>(decodedData[(y * width) + x]);
            auto px = palette[b];
            image[y][x] = png::rgb_pixel(px.red, px.green, px.blue);
        }
    }

    image.write(outFile);

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Specify palette, input and output files" << std::endl;
        return 1;
    }

    std::string inFile(argv[1]);
    std::string outFile(argv[2]);

    return convert(inFile, outFile);
}
