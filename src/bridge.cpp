#include <iostream>
#include <string>
#include <rwe/_3do.h>
#include <rwe/fixed_point.h>
#include <rwe/optional_io.h>
#include <vector>
#include <rwe/rwe_string.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>
#include <nlohmann/json.hpp>

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

void writeMapSuccess(const std::string& data)
{
    json j = {
        {"result", "ok"},
        {"data", data},
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
            std::string dataString;
            dataString.insert(dataString.begin(), data->begin(), data->end());
            writeMapSuccess(dataString);
        }
        else if (command == "map-list")
        {
            auto mapNames = getMapNames(vfs);
            writeMapListSuccess(mapNames);
        }
        else
        {
            std::cout << "Unrecognised command" << std::endl;
            return 1;
        }
    }

    return 0;
}
