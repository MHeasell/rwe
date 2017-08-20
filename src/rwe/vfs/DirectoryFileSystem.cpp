#include "DirectoryFileSystem.h"

#include <fstream>
#include <sstream>

namespace fs = boost::filesystem;

namespace rwe
{
    boost::optional<std::vector<char>> DirectoryFileSystem::readFile(const std::string& filename) const
    {
        fs::path fullPath;
        fullPath /= path;
        fullPath /= filename;

        std::ifstream input(fullPath.string(), std::ios::binary);
        if (!input.is_open())
        {
            return boost::none;
        }

        std::ostringstream buf;
        buf << input.rdbuf();
        const auto& str = buf.str();

        std::vector<char> output(str.length());
        std::copy(str.begin(), str.end(), output.begin());

        return output;
    }

    DirectoryFileSystem::DirectoryFileSystem(const std::string& path)
        : path(path)
    {
    }
}
