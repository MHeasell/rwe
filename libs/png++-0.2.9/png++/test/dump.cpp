#include <iostream>
#include <ostream>

#include <png.hpp>

// inline png::uint_16
// to_png_order(png::uint_16 x)
// {
//     return ((x & 0xff) << 8) | (x >> 8);
// }

int
main(int argc, char* argv[])
try
{
    if (argc != 2)
    {
        throw std::runtime_error("usage: dump PNG");
    }
    char const* file = argv[1];
    png::image< png::gray_pixel_16 > image(file);
    for (size_t y = 0; y < image.get_height(); ++y)
    {
        for (size_t x = 0; x < image.get_width(); ++x)
        {
            if (x)
            {
                putchar(' ');
            }
            printf("%04x", image[y][x]);
        }
        putchar('\n');
    }
}
catch (std::exception const& error)
{
    std::cerr << "dump: " << error.what() << std::endl;
    return EXIT_FAILURE;
}
