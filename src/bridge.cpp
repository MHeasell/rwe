#include <SDL.h>
#include <boost/filesystem.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <png++/png.hpp>
#include <rwe/_3do.h>
#include <rwe/fixed_point.h>
#include <rwe/optional_io.h>
#include <rwe/ota.h>
#include <rwe/rwe_string.h>
#include <rwe/tdf.h>
#include <rwe/tnt/TntArchive.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>
#include <string>
#include <vector>

namespace fs = boost::filesystem;

using json = nlohmann::json;

bool hasMultiplayerSchema(rwe::CompositeVirtualFileSystem& vfs, const std::string& source, const std::string& mapName)
{
    auto otaRaw = vfs.readFileFromSource(source, std::string("maps/").append(mapName).append(".ota"));
    if (!otaRaw)
    {
        throw std::runtime_error("Failed to read OTA file");
    }

    std::string otaStr(otaRaw->begin(), otaRaw->end());
    auto ota = parseOta(rwe::parseTdfFromString(otaStr));

    return std::any_of(ota.schemas.begin(), ota.schemas.end(), [](const auto& s) { return rwe::startsWith(rwe::toUpper(s.type), "NETWORK"); });
}

std::vector<std::pair<std::string, std::string>> getMapNames(rwe::CompositeVirtualFileSystem& vfs)
{
    auto mapNames = vfs.getFileNamesWithSources("maps", ".ota");

    for (auto& e : mapNames)
    {
        // chop off the extension
        e.first.resize(e.first.size() - 4);
    }

    // Keep only maps that have a multiplayer schema
    mapNames.erase(std::remove_if(mapNames.begin(), mapNames.end(), [&vfs](const auto& e) { return !hasMultiplayerSchema(vfs, e.second, e.first); }), mapNames.end());

    return mapNames;
}

void writeError(const std::string& error)
{
    json j = {
        {"result", "error"},
        {"message", error}};
    std::cout << j << std::endl;
}

void writeSuccess()
{
    json j = {
        {"result", "ok"},
    };
    std::cout << j << std::endl;
}

void writeMapListSuccess(const std::vector<std::pair<std::string, std::string>>& names)
{
    std::vector<json> mapsJson;
    std::transform(names.begin(), names.end(), std::back_inserter(mapsJson), [](const auto& m) {
        return json{{"name", m.first}, {"source", m.second}};
    });
    json j = {
        {"result", "ok"},
        {"maps", mapsJson},
    };
    std::cout << j << std::endl;
}

void writeGetMinimapSuccess(const std::string& outputFileName)
{
    json j = {
        {"result", "ok"},
        {"path", outputFileName},
    };
    std::cout << j << std::endl;
}

void writeMapInfoSuccess(const rwe::OtaRecord& ota)
{
    json j = {
        {"result", "ok"},
        {"description", ota.missionDescription},
        {"memory", ota.memory},
        {"numberOfPlayers", ota.numPlayers},
    };
    std::cout << j << std::endl;
}

void writeGetModeListSuccess(const std::vector<std::pair<int, int>>& modes)
{
    std::vector<json> modesJson;
    std::transform(modes.begin(), modes.end(), std::back_inserter(modesJson), [](const auto& m) {
        return json{{"width", m.first}, {"height", m.second}};
    });
    json j = {
        {"modes", modesJson},
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

    void extractMinimap(CompositeVirtualFileSystem& vfs, const png::rgb_pixel* palette, const std::string& source, const std::string& mapName, const std::string& outputPath)
    {

        auto tntData = vfs.readFileFromSource(source, "maps/" + mapName + ".tnt");
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

std::optional<rwe::OtaRecord> getMapInfo(rwe::CompositeVirtualFileSystem& vfs, const std::string& source, const std::string& mapName)
{
    auto data = vfs.readFileFromSource(source, "maps/" + mapName + ".ota");
    if (!data)
    {
        return std::nullopt;
    }
    std::string dataString(data->begin(), data->end());
    return rwe::parseOta(rwe::parseTdfFromString(dataString));
}

std::optional<std::string> getMinimap(rwe::CompositeVirtualFileSystem& vfs, const std::string& source, const std::string& mapName)
{
    auto paletteBytes = vfs.readFile("palettes/PALETTE.PAL");
    if (!paletteBytes)
    {
        return std::nullopt;
    }

    boost::interprocess::bufferstream paletteBuffer(paletteBytes->data(), paletteBytes->size());
    png::rgb_pixel palette[256];
    rwe::loadPalette(paletteBuffer, palette);

    auto output = fs::temp_directory_path();
    output /= mapName + ".png";

    rwe::extractMinimap(vfs, palette, source, mapName, output.string());
    return output.string();
}

std::optional<std::vector<std::pair<int, int>>> getVideoModes()
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        return std::nullopt;
    }
    std::vector<std::pair<int, int>> modeList;
    auto displayModes = SDL_GetNumDisplayModes(0);
    for (int i = 0; i < displayModes; ++i)
    {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) != 0)
        {
            continue;
        }
        if (mode.format != SDL_PIXELFORMAT_RGB888 || mode.refresh_rate != 60)
        {
            continue;
        }
        modeList.emplace_back(mode.w, mode.h);
    }
    SDL_Quit();
    return modeList;
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
            auto sourceIt = j.find("source");
            if (sourceIt == j.end())
            {
                std::cout << "Missing source field" << std::endl;
                return 1;
            }

            auto ota = getMapInfo(vfs, sourceIt->get<std::string>(), mapIt->get<std::string>());
            if (ota)
            {
                writeMapInfoSuccess(*ota);
            }
            else
            {
                std::cout << "Map ota not found!" << std::endl;
                return 1;
            }
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
            auto sourceIt = j.find("source");
            if (sourceIt == j.end())
            {
                std::cout << "Missing source field" << std::endl;
                return 1;
            }

            auto filename = getMinimap(vfs, sourceIt->get<std::string>(), mapIt->get<std::string>());
            if (filename)
            {
                writeGetMinimapSuccess(*filename);
            }
            else
            {
                std::cout << "Failed to load minimap" << std::endl;
                return 1;
            }
        }
        else if (command == "video-modes")
        {
            auto modeList = getVideoModes();
            if (modeList)
            {
                writeGetModeListSuccess(*modeList);
            }
            else
            {
                std::cout << "Failed to get video modes" << std::endl;
                return 1;
            }
        }
        else
        {
            std::cout << "Unrecognised command" << std::endl;
            return 1;
        }
    }

    return 0;
}
