#include <rwe/rwe_string.h>
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

    std::vector<std::string> HpiFileSystem::getFileNames(const std::string& directory, const std::string& extension)
    {
        auto dir = hpi.findDirectory(directory);
        if (!dir)
        {
            // no such directory, so no files inside it
            return std::vector<std::string>();
        }

        auto it = dir->entries.begin();
        auto end = dir->entries.end();

        std::vector<std::string> v;

        for (; it != end; ++it)
        {
            const auto& e = *it;

            if (endsWith(e.name, extension))
            {
                v.push_back(e.name);
            }
        }

        return v;
    }
}
