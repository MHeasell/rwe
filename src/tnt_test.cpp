#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <png++/png.hpp>
#include <rwe/Gaf.h>
#include <rwe/tnt/TntArchive.h>
#include <string>

namespace fs = boost::filesystem;

void loadPalette(const std::string& filename, png::rgb_pixel* buffer)
{
    std::ifstream in(filename, std::ios::binary);

    for (unsigned int i = 0; i < 256; ++i)
    {
        in.read(reinterpret_cast<char*>(&(buffer[i].red)), 1);
        in.read(reinterpret_cast<char*>(&(buffer[i].green)), 1);
        in.read(reinterpret_cast<char*>(&(buffer[i].blue)), 1);
        in.seekg(1, std::ios::cur); // skip alpha
    }
}

int featuresCommand(const std::string& tntPath)
{
    std::ifstream file(tntPath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << tntPath << std::endl;
        return 1;
    }

    rwe::TntArchive tnt(&file);

    tnt.readFeatures([](const std::string& featureName) {
        std::cout << featureName << std::endl;
    });

    return 0;
}

int tilesCommand(const std::string& palettePath, const std::string& tntPath, const std::string& outputPath)
{
    png::rgb_pixel palette[256];
    loadPalette(palettePath, palette);

    std::ifstream file(tntPath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << tntPath << std::endl;
        return 1;
    }

    rwe::TntArchive tnt(&file);

    fs::path outPath(outputPath);

    int i = 0;
    tnt.readTiles([&palette, &i, &outPath](const char* tileData) {

        png::image<png::rgb_pixel> image(32, 32);
        for (png::uint_32 y = 0; y < image.get_height(); ++y)
        {
            for (png::uint_32 x = 0; x < image.get_width(); ++x)
            {
                auto b = static_cast<unsigned char>(tileData[(y * 32) + x]);
                assert(b >= 0 && b < 256);
                auto px = palette[b];
                image[y][x] = px;
            }
        }

        fs::path out(outPath);
        out /= std::to_string(i++) + ".png";

        image.write(out.string());
    });

    return 0;
}

int infoCommand(const std::string& tntPath)
{
    std::ifstream file(tntPath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << tntPath << std::endl;
        return 1;
    }

    rwe::TntArchive tnt(&file);
    const auto& header = tnt.getHeader();

    std::cout << "Width: " << header.width << std::endl;
    std::cout << "Height: " << header.height << std::endl;
    std::cout << "SeaLevel: " << header.seaLevel << std::endl;
    std::cout << "Tiles: " << header.numberOfTiles << std::endl;
    std::cout << "Features: " << header.numberOfFeatures << std::endl;

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Specify a command" << std::endl;
        return 1;
    }

    std::string command(argv[1]);

    if (command == "info")
    {
        if (argc < 3)
        {
            std::cerr << "Specify a TNT file" << std::endl;
            return 1;
        }

        std::string tntFile(argv[2]);

        return infoCommand(tntFile);
    }

    if (command == "features")
    {
        if (argc < 3)
        {
            std::cerr << "Specify a TNT file" << std::endl;
            return 1;
        }

        std::string tntFile(argv[2]);

        return featuresCommand(tntFile);
    }

    if (command == "tiles")
    {
        if (argc < 5)
        {
            std::cerr << "Specify a palette file, tnt file and output directory" << std::endl;
            return 1;
        }

        std::string paletteFile(argv[2]);
        std::string tntFile(argv[3]);
        std::string destPath(argv[4]);

        return tilesCommand(paletteFile, tntFile, destPath);
    }

    std::cerr << "Unrecognised command: " << command << std::endl;
    return 1;
}
