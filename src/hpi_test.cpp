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
            return "ZlLib";
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

int main(int argc, char* argv[])
{
    std::string filename(argv[1]);
    std::cout << "HPI archive: " << filename << std::endl;
    std::ifstream file(filename, std::ios::binary);

    std::cout << "Opening..." << std::endl;
    rwe::HpiArchive archive(&file);

    std::cout << "Enumerating contents..." << std::endl;
    printDir(0, "<ROOT>", archive.root());
}
