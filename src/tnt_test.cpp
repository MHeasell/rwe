#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <rwe/io/gaf/GafArchive.h>
#include <rwe/io/tnt/TntArchive.h>
#include <rwe/util/png_write.h>
#include <string>

namespace fs = std::filesystem;

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
    rwe::RgbPixel palette[256];
    rwe::loadPalette(palettePath, palette);

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
        rwe::PngImage image(32, 32);
        for (uint32_t y = 0; y < 32; ++y)
        {
            for (uint32_t x = 0; x < 32; ++x)
            {
                auto b = static_cast<unsigned char>(tileData[(y * 32) + x]);
                image.at(x, y) = palette[b];
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

int minimapCommand(const std::string& palettePath, const std::string& tntPath, const std::string& outputPath)
{
    rwe::RgbPixel palette[256];
    rwe::loadPalette(palettePath, palette);

    std::ifstream file(tntPath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << tntPath << std::endl;
        return 1;
    }

    rwe::TntArchive tnt(&file);
    auto minimap = tnt.readMinimap();

    rwe::PngImage image(minimap.width, minimap.height);
    for (uint32_t y = 0; y < minimap.height; ++y)
    {
        for (uint32_t x = 0; x < minimap.width; ++x)
        {
            auto b = static_cast<unsigned char>(minimap.data[(y * minimap.width) + x]);
            image.at(x, y) = palette[b];
        }
    }

    image.write(outputPath);

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

    if (command == "minimap")
    {
        if (argc < 5)
        {
            std::cerr << "Specify a palette file, tnt file and output directory" << std::endl;
            return 1;
        }

        std::string paletteFile(argv[2]);
        std::string tntFile(argv[3]);
        std::string destPath(argv[4]);

        return minimapCommand(paletteFile, tntFile, destPath);
    }

    std::cerr << "Unrecognised command: " << command << std::endl;
    return 1;
}
