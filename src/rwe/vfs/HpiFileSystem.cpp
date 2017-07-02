#include "HpiFileSystem.h"

namespace rwe
{
    boost::optional<std::vector<char>> HpiFileSystem::readFile(const std::string& filename) const
    {
        auto file = hpi.findFile(filename);
        if (!file)
        {
            return boost::none;
        }

        std::vector<char> buffer(file->size);
        hpi.extract(*file, buffer.data());

        return buffer;
    }

    HpiFileSystem::HpiFileSystem(const std::string& file)
        : stream(file, std::ios::binary),
          hpi(&stream)
    {
        if (!stream.is_open())
        {
            throw std::runtime_error("Could not open file");
        }
    }
}
