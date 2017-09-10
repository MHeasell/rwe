#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <png++/png.hpp>
#include <rwe/Gaf.h>
#include <rwe/tnt/TntArchive.h>
#include <string>

namespace fs = boost::filesystem;

int listFeatures(const std::string& tntPath)
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

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Specify a command" << std::endl;
        return 1;
    }

    std::string command(argv[1]);

    if (command == "features")
    {
        if (argc < 3)
        {
            std::cerr << "Specify a TNT file" << std::endl;
            return 1;
        }

        std::string tntFile(argv[2]);

        return listFeatures(tntFile);

    }

    std::cerr << "Unrecognised command: " << command << std::endl;
    return 1;
}
