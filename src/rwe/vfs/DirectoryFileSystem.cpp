#include "DirectoryFileSystem.h"

#include <fstream>
#include <sstream>

namespace fs = boost::filesystem;

namespace rwe
{
    std::optional<std::vector<char>> DirectoryFileSystem::readFile(const std::string& filename) const
    {
        fs::path fullPath;
        fullPath /= path;
        fullPath /= filename;

        std::ifstream input(fullPath.string(), std::ios::binary);
        if (!input.is_open())
        {
            return std::nullopt;
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

    DirectoryFileSystem::DirectoryFileSystem(const boost::filesystem::path& path)
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
                v.push_back(filename.string());
            }
        }

        return v;
    }

    std::vector<std::string>
    DirectoryFileSystem::getFileNamesRecursive(const std::string& directory, const std::string& extension)
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
            if (e.status().type() == fs::file_type::directory_file)
            {
                // recurse into directory
                auto innerDirectoryName = e.path().filename();
                auto innerPath = fs::path(directory) / innerDirectoryName;
                auto innerEntries = getFileNamesRecursive(innerPath.string(), extension);
                for (const auto& innerEntry : innerEntries)
                {
                    auto path = fs::path(innerDirectoryName) / innerEntry;
                    v.push_back(path.string());
                }
            }
            else if (e.path().extension().string() == extension)
            {
                auto filename = e.path().filename();
                v.push_back(filename.string());
            }
        }

        return v;
    }
}
