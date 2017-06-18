#include <rwe/Hpi.h>
#include <string>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
    std::string filename(argv[1]);
    std::cout << "HPI archive: " << filename << std::endl;
    std::ifstream file(filename, std::ios::binary);

    std::cout << "Opening..." << std::endl;
    rwe::HpiArchive archive(&file);

    std::cout << "Enumerating contents..." << std::endl;
    for (const auto& entry : archive)
    {
        const char* name = archive.getData() + entry.nameOffset;
        std::cout << name << " ";
        auto type = entry.isDirectory != 0 ? "directory" : "file";
        std::cout << type << std::endl;
    }
}
