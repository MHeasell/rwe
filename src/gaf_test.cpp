#include <rwe/Gaf.h>
#include <string>
#include <fstream>
#include <iostream>

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

int extractCommand(const std::string& gafPath, const std::string& entryName, const std::string& destinationPath)
{
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

    throw std::logic_error("Not implemented.");
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
        if (argc < 5)
        {
            std::cerr << "Specify GAF file, file to extract and destination" << std::endl;
            return 1;
        }

        return extractCommand(argv[2], argv[3], argv[4]);
    }

    std::cerr << "Unrecognised command: " << command << std::endl;
    return 1;
}
