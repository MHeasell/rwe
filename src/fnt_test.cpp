#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <png++/png.hpp>
#include <rwe/io/fnt/Fnt.h>
#include <rwe/optional_io.h>
#include <vector>


void renderFontFile(std::istream& in, std::ostream& out)
{
    rwe::FntArchive fnt(&in);

    std::vector<char> v(512);

    png::image<png::rgb_pixel> image(256, 512);
    for (int i = 0; i < 256; ++i)
    {
        int charX = (i % 16) * 16;
        int charY = (i / 16) * 32;

        int byteLength = fnt.extract(i, v.data());
        int width = byteLength / 2;
        int bitLength = byteLength * 8;

        for (int j = 0; j < bitLength; ++j)
        {
            int dx = j % width;
            int dy = j / width;
            if (dy >= fnt.glyphHeight())
            {
                break;
            }

            auto val = (static_cast<unsigned char>(v[j / 8u]) >> (7u - (j % 8u))) & 1u;
            auto pxVal = val ? 255 : 0;
            image[charY + dy][charX + dx] = png::rgb_pixel(pxVal, pxVal, pxVal);
        }
    }

    image.write_stream(out);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Specify a command" << std::endl;
        return 1;
    }

    std::string cmd(argv[1]);

    if (cmd == "render")
    {
        if (argc < 4)
        {
            std::cerr << "Specify a FNT file to dump." << std::endl;
            return 1;
        }

        std::string filename(argv[2]);
        std::string outFilename(argv[3]);

        std::ifstream fh(filename, std::ios::binary);
        std::ofstream ofh(outFilename, std::ios::binary);

        renderFontFile(fh, ofh);
        return 0;
    }

    std::cerr << "Unrecognised command" << std::endl;
    return 1;
}
