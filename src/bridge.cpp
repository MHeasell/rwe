#include <iostream>
#include <string>
#include <rwe/_3do.h>
#include <rwe/fixed_point.h>
#include <rwe/optional_io.h>
#include <vector>
#include <rwe/rwe_string.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>
#include <nlohmann/json.hpp>
#include <rwe/tnt/TntArchive.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <png++/png.hpp>
#include <boost/filesystem.hpp>
#include <rwe/ota.h>
#include <rwe/tdf.h>

namespace fs = boost::filesystem;

using json = nlohmann::json;

std::vector<std::string> getMapNames(rwe::AbstractVirtualFileSystem& vfs)
{
    auto mapNames = vfs.getFileNames("maps", ".ota");

    for (auto& e : mapNames)
    {
        // chop off the extension
        e.resize(e.size() - 4);
    }

    return mapNames;
}

void writeError(const std::string& error)
{
    json j = {
        {"result", "error"},
        {"message", error}
    };
    std::cout << j << std::endl;
}

void writeSuccess()
{
    json j = {
        {"result", "ok"},
    };
    std::cout << j << std::endl;
}

void writeMapListSuccess(const std::vector<std::string>& names) {
    json j = {
        {"result", "ok"},
        {"maps", names},
    };
    std::cout << j << std::endl;
}

void writeGetMinimapSuccess(const std::string& outputFileName) {
    json j = {
        {"result", "ok"},
        {"path", outputFileName},
    };
    std::cout << j << std::endl;
}

void writeMapInfoSuccess(const rwe::OtaRecord& ota) {
    json j = {
        {"result", "ok"},
        {"description", ota.missionDescription},
        {"memory", ota.memory},
        {"numberOfPlayers", ota.numPlayers},
    };
    std::cout << j << std::endl;
}

namespace rwe
{
    void loadPalette(std::istream& in, png::rgb_pixel* buffer)
    {
        for (unsigned int i = 0; i < 256; ++i)
        {
            in.read(reinterpret_cast<char*>(&(buffer[i].red)), 1);
            in.read(reinterpret_cast<char*>(&(buffer[i].green)), 1);
            in.read(reinterpret_cast<char*>(&(buffer[i].blue)), 1);
            in.seekg(1, std::ios::cur); // skip alpha
        }
    }

    void extractMinimap(AbstractVirtualFileSystem& vfs, const png::rgb_pixel* palette, const std::string& mapName, const std::string& outputPath)
    {

        auto tntData = vfs.readFile("maps/" + mapName + ".tnt");
        if (!tntData)
        {
            throw std::runtime_error("map tnt not found!");
        }

        boost::interprocess::bufferstream tntStream(tntData->data(), tntData->size());
        TntArchive tnt(&tntStream);
        auto minimap = tnt.readMinimap();

        png::image<png::rgb_pixel> image(minimap.width, minimap.height);
        for (png::uint_32 y = 0; y < image.get_height(); ++y)
        {
            for (png::uint_32 x = 0; x < image.get_width(); ++x)
            {
                auto b = static_cast<unsigned char>(minimap.data[(y * minimap.width) + x]);
                assert(b >= 0 && b < 256);
                auto px = palette[b];
                image[y][x] = px;
            }
        }

        image.write(outputPath);
    }
}

int main(int argc, char* argv[])
{
    rwe::CompositeVirtualFileSystem vfs;
    json j;

    while (std::cin >> j)
    {
        auto cmdIt = j.find("command");
        if (cmdIt == j.end())
        {
            std::cout << "Missing command field" << std::endl;
            return 1;
        }

        std::string command = *cmdIt;
        if (command == "clear-data-paths")
        {
            vfs.clear();
            writeSuccess();
        }
        else if (command == "add-data-path")
        {
            auto pathIt = j.find("path");
            if (pathIt == j.end())
            {
                std::cout << "Missing path field" << std::endl;
                return 1;
            }

            rwe::addToVfs(vfs, pathIt->get<std::string>());
            writeSuccess();
        }
        else if (command == "map-info")
        {
            auto mapIt = j.find("map");
            if (mapIt == j.end())
            {
                std::cout << "Missing map field" << std::endl;
                return 1;
            }

            auto data = vfs.readFile("maps/" + mapIt->get<std::string>() + ".ota");
            if (!data)
            {
                std::cout << "Map ota not found!" << std::endl;
                return 1;
            }
            std::string dataString(data->begin(), data->end());
            auto ota = rwe::parseOta(rwe::parseTdfFromString(dataString));
            writeMapInfoSuccess(ota);
        }
        else if (command == "map-list")
        {
            auto mapNames = getMapNames(vfs);
            writeMapListSuccess(mapNames);
        }
        else if (command == "get-minimap")
        {
            auto mapIt = j.find("map");
            if (mapIt == j.end())
            {
                std::cout << "Missing map field" << std::endl;
                return 1;
            }
            auto paletteBytes = vfs.readFile("palettes/PALETTE.PAL");
            if (!paletteBytes)
            {
                throw std::runtime_error("Couldn't find palette");
            }

            boost::interprocess::bufferstream paletteBuffer(paletteBytes->data(), paletteBytes->size());
            png::rgb_pixel palette[256];
            rwe::loadPalette(paletteBuffer, palette);

            auto mapName = mapIt->get<std::string>();

            auto output = fs::temp_directory_path();
            output /= mapName + ".png";

            rwe::extractMinimap(vfs, palette, mapName, output.string());
            writeGetMinimapSuccess(output.string());
        }
        else
        {
            std::cout << "Unrecognised command" << std::endl;
            return 1;
        }
    }

    return 0;
}
