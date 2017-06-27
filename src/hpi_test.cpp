#include <rwe/Hpi.h>
#include <string>
#include <fstream>
#include <iostream>

namespace rwe
{
    class NameVisitor : public boost::static_visitor<std::string>
    {
    public:
        std::string operator()(const HpiArchive::File& i) const { return "file"; }
        std::string operator()(const HpiArchive::Directory& i) const { return "directory"; }
    };
}

std::string schemeName(rwe::HpiArchive::File::CompressionScheme scheme)
{
    switch (scheme)
    {
        case rwe::HpiArchive::File::CompressionScheme::None:
            return "None";
        case rwe::HpiArchive::File::CompressionScheme::LZ77:
            return "LZ77";
        case rwe::HpiArchive::File::CompressionScheme::ZLib:
            return "ZLib";
        default:
            throw std::runtime_error("Invalid compression scheme");
    }
}

void printDir(unsigned int indent, const std::string& name, const rwe::HpiArchive::Directory& d);
void printFile(unsigned int indent, const std::string& name, const rwe::HpiArchive::File& f);

void printEntry(unsigned int indent, const rwe::HpiArchive::DirectoryEntry& e)
{
    if (const auto f = boost::get<rwe::HpiArchive::File>(&e.data))
    {
        printFile(indent, e.name, *f);
    }
    else if (const auto d = boost::get<rwe::HpiArchive::Directory>(&e.data))
    {
        printDir(indent, e.name, *d);
    }
}

void printDir(unsigned int indent, const std::string& name, const rwe::HpiArchive::Directory& d)
{
    std::string dent(indent, ' ');
    std::cout << dent << name << " directory:" << std::endl;
    for (const auto& entry : d.entries)
    {
        printEntry(indent + 4, entry);
    }
}

void printFile(unsigned int indent, const std::string& name, const rwe::HpiArchive::File& f)
{
    std::string compression = schemeName(f.compressionScheme);

    std::string dent(indent, ' ');
    std::cout << dent << name << " file " << compression << std::endl;
}

int listCommand(const std::string& filename)
{
    std::cout << "HPI archive: " << filename << std::endl;
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    std::cout << "Opening..." << std::endl;
    rwe::HpiArchive archive(&file);

    std::cout << "Enumerating contents..." << std::endl;
    printDir(0, "<ROOT>", archive.root());

    return 0;
}

int extractCommand(const std::string& hpiPath, const std::string& filePath, const std::string& destinationPath)
{
    std::cout << "HPI archive: " << hpiPath << std::endl;
    std::ifstream file(hpiPath, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    std::cout << "Opening..." << std::endl;
    rwe::HpiArchive archive(&file);

    std::cout << "Finding file..." << std::endl;
    auto entry = archive.findFile(filePath);
    if (!entry)
    {
        std::cerr << "Could not find file '" << filePath << "' inside archive." << std::endl;
        return 1;
    }

    std::cout << "Extracting..." << std::endl;
    auto buf = std::make_unique<char[]>(entry->size);
    archive.extract(*entry, buf.get());

    std::cout << "Writing..." << std::endl;
    std::ofstream out(destinationPath, std::ios::binary);
    out.write(buf.get(), entry->size);

    std::cout << "Done!" << std::endl;

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
            std::cerr << "Specify a HPI file to list" << std::endl;
            return 1;
        }

        return listCommand(argv[2]);
    }

    if (command == "extract")
    {
        if (argc < 5)
        {
            std::cerr << "Specify HPI file, file to extract and destination" << std::endl;
            return 1;
        }

        return extractCommand(argv[2], argv[3], argv[4]);
    }

    std::cerr << "Unrecognised command: " << command << std::endl;
    return 1;
}
