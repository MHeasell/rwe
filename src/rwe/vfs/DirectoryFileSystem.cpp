#include "DirectoryFileSystem.h"

#include <fstream>
#include <rwe/rwe_string.h>
#include <sstream>

namespace fs = boost::filesystem;

namespace rwe
{
    /**
     * Searches the directory tree for the given path
     * in a case-insensitive manner and returns the path with the correct case.
     * The search starts from the root, which is assumed
     * to already be correctly cased.
     *
     * This function is necessary because file references in TA
     * are case-insensitive, however some of the filesystems
     * we may be operating on (e.g. Linux) are case-sensitive.
     * If we naively try to follow file references on a case-sensitive filesystem
     * we may fail to find the file we wanted.
     */
    std::optional<boost::filesystem::path> findPathCaseInsensitive(
        const boost::filesystem::path& root, const boost::filesystem::path& path)
    {
        fs::path basePath;

        for (const auto& component : path)
        {
            fs::path searchPath(root);
            searchPath /= basePath;
            fs::directory_iterator it(searchPath);
            fs::directory_iterator end;

            auto foundItem = std::find_if(it, end, [&component](const auto& e) {
                return toUpper(e.path().filename().string()) == toUpper(component.string());
            });

            if (foundItem == end)
            {
                return std::nullopt;
            }

            basePath /= foundItem->path().filename();
        }

        return basePath;
    }

    DirectoryFileSystem::DirectoryFileSystem(const std::string& path)
        : path(path)
    {
    }

    DirectoryFileSystem::DirectoryFileSystem(const boost::filesystem::path& path)
        : path(path)
    {
    }

    std::optional<std::vector<char>> DirectoryFileSystem::readFile(const std::string& filename) const
    {
        fs::path fullPath(path);
        auto correctlyCasedPath = findPathCaseInsensitive(path, filename);
        if (!correctlyCasedPath)
        {
            return std::nullopt;
        }

        fullPath /= *correctlyCasedPath;

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

            if (toUpper(e.path().extension().string()) == toUpper(extension))
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
            else if (toUpper(e.path().extension().string()) == toUpper(extension))
            {
                auto filename = e.path().filename();
                v.push_back(filename.string());
            }
        }

        return v;
    }
}
