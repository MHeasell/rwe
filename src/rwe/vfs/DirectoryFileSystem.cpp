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

    std::vector<std::string> DirectoryFileSystem::getFileNames(const std::string& directory, const std::string& extension)
    {
        fs::path fullPath;
        fullPath /= path;
        fullPath /= directory;

        std::vector<std::string> v;

        // FIXME: TOCTOU error here
        if (!fs::exists(fullPath))
        {
            return std::vector<std::string>();
        }

        fs::directory_iterator it(fullPath);
        fs::directory_iterator end;

        for (; it != end; ++it)
        {
            const auto& e = *it;

            if (e.path().extension().string() == extension)
            {
                auto filename = e.path().filename();
                filename.replace_extension();
                v.push_back(filename.string());
            }
        }

        return v;
    }
}
