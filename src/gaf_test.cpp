#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <png++/png.hpp>
#include <rwe/io/gaf/Gaf.h>
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


int listCommand(const std::string& filename)
{
    std::cout << "GAF archive: " << filename << std::endl;
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    std::cout << "Opening..." << std::endl;
    rwe::GafArchive archive(&file);

    std::cout << "Enumerating contents..." << std::endl;

    for (auto& entry : archive.entries())
    {
        std::cout << entry.name << ", " << entry.frameOffsets.size() << " frames" << std::endl;
    }

    return 0;
}

class GafAdapter : public rwe::GafReaderAdapter
{
private:
    png::rgb_pixel* palette;
    std::size_t frameCount;
    std::unique_ptr<char[]> currentFrame;
    rwe::GafFrameData currentFrameHeader;
    fs::path destPath;

public:
    explicit GafAdapter(png::rgb_pixel* palette, const std::string& destPath) : palette(palette), frameCount(0), currentFrame(), destPath(destPath) {}
    void beginFrame(const rwe::GafFrameEntry& entry, const rwe::GafFrameData& header) override
    {
        std::cout << "Beginning frame " << frameCount << std::endl;
        currentFrameHeader = header;
        std::cout << "Frame entry data unknown1: " << entry.unknown1 << std::endl;
        std::cout << "Frame info: width: " << header.width << ", height: " << header.height << ", posX: " << header.posX << ", posY: " << header.posY << ", transparencyIndex: " << static_cast<int>(header.transparencyIndex) << ", compressed: " << static_cast<int>(header.compressed) << ", subframe count: " << header.subframesCount << ", unknown2: " << header.unknown2 << ", unknown 3: " << header.unknown3 << std::endl;
        currentFrame = std::make_unique<char[]>(header.width * header.height);
        std::fill_n(currentFrame.get(), header.width * header.height, header.transparencyIndex);
    }

    void frameLayer(const LayerData& data) override
    {
        // copy the layer onto the frame
        for (std::size_t y = 0; y < data.height; ++y)
        {
            for (std::size_t x = 0; x < data.width; ++x)
            {
                auto outPosX = static_cast<int>(x) - (data.x - currentFrameHeader.posX);
                auto outPosY = static_cast<int>(y) - (data.y - currentFrameHeader.posY);

                if (outPosX < 0 || outPosX >= currentFrameHeader.width || outPosY < 0 || outPosY >= currentFrameHeader.height)
                {
                    throw std::runtime_error("frame coordinate out of bounds");
                }

                auto colorIndex = data.data[(y * data.width) + x];
                if (colorIndex == currentFrameHeader.transparencyIndex)
                {
                    continue;
                }

                currentFrame[(outPosY * currentFrameHeader.width) + outPosX] = colorIndex;
            }
        }
    }

    void endFrame() override
    {
        png::image<png::rgb_pixel> image(currentFrameHeader.width, currentFrameHeader.height);
        for (png::uint_32 y = 0; y < image.get_height(); ++y)
        {
            for (png::uint_32 x = 0; x < image.get_width(); ++x)
            {
                auto b = static_cast<unsigned char>(currentFrame[(y * currentFrameHeader.width) + x]);
                assert(b >= 0 && b < 256);
                auto px = palette[b];
                image[y][x] = px;
            }
        }

        fs::path fullPath = destPath / std::to_string(frameCount).append(".png");

        image.write(fullPath.string());
        std::cout << "Finished frame " << frameCount << std::endl;
        ++frameCount;
    }
};

int extractCommand(const std::string& palettePath, const std::string& gafPath, const std::string& entryName, const std::string& destinationPath)
{
    std::cout << "Palette file: " << palettePath << std::endl;

    png::rgb_pixel palette[256];
    loadPalette(palettePath, palette);

    std::cout << "GAF archive: " << gafPath << std::endl;
    std::ifstream file(gafPath, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    std::cout << "Opening..." << std::endl;
    rwe::GafArchive archive(&file);

    std::cout << "Finding file..." << std::endl;
    auto entry = archive.findEntry(entryName);
    if (!entry)
    {
        std::cerr << "Could not find entry '" << entryName << "' inside archive." << std::endl;
        return 1;
    }

    std::cout << "Extracting..." << std::endl;

    GafAdapter adapter(palette, destinationPath);
    archive.extract(*entry, adapter);

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

    if (command == "list")
    {
        if (argc < 3)
        {
            std::cerr << "Specify a GAF file to list" << std::endl;
            return 1;
        }

        return listCommand(argv[2]);
    }

    if (command == "extract")
    {
        if (argc < 6)
        {
            std::cerr << "Specify palette file, GAF file, file to extract and destination" << std::endl;
            return 1;
        }

        return extractCommand(argv[2], argv[3], argv[4], argv[5]);
    }

    std::cerr << "Unrecognised command: " << command << std::endl;
    return 1;
}
