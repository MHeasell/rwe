#include "CompositeVirtualFileSystem.h"
#include <boost/filesystem.hpp>
#include <map>
#include <rwe/rwe_string.h>
#include <rwe/vfs/DirectoryFileSystem.h>
#include <rwe/vfs/HpiFileSystem.h>
#include <set>

namespace fs = boost::filesystem;

namespace rwe
{
    std::optional<std::vector<char>> CompositeVirtualFileSystem::readFile(const std::string& filename) const
    {
        for (const auto& fs : filesystems)
        {
            auto file = fs->readFile(filename);
            if (file)
            {
                return file;
            }
        }

        return std::nullopt;
    }

    std::optional<std::vector<char>> CompositeVirtualFileSystem::readFileFromSource(const std::string& source, const std::string& filename) const
    {
        for (const auto& fs : filesystems)
        {
            if (fs->getPath() != source)
            {
                continue;
            }

            auto file = fs->readFile(filename);
            if (file)
            {
                return file;
            }
            return std::nullopt;
        }

        return std::nullopt;
    }

    std::vector<std::string>
    CompositeVirtualFileSystem::getFileNames(const std::string& directory, const std::string& extension)
    {
        std::set<std::string> entries;

        for (const auto& fs : filesystems)
        {
            auto v = fs->getFileNames(directory, extension);
            entries.insert(v.begin(), v.end());
        }

        std::vector<std::string> v(entries.begin(), entries.end());
        return v;
    }

    std::vector<std::pair<std::string, std::string>>
    CompositeVirtualFileSystem::getFileNamesWithSources(const std::string& directory, const std::string& extension)
    {
        std::map<std::string, std::string> entries;

        for (const auto& fs : filesystems)
        {
            auto v = fs->getFileNames(directory, extension);
            for (const auto& filename : v)
            {
                entries.insert({filename, fs->getPath()});
            }
        }

        std::vector<std::pair<std::string, std::string>> v(entries.begin(), entries.end());
        return v;
    }

    std::vector<std::string>
    CompositeVirtualFileSystem::getFileNamesRecursive(const std::string& directory, const std::string& extension)
    {
        std::set<std::string> entries;

        for (const auto& fs : filesystems)
        {
            auto v = fs->getFileNamesRecursive(directory, extension);
            entries.insert(v.begin(), v.end());
        }

        std::vector<std::string> v(entries.begin(), entries.end());
        return v;
    }

    std::vector<std::pair<std::string, std::string>>
    CompositeVirtualFileSystem::getFileNamesRecursiveWithSources(const std::string& directory, const std::string& extension)
    {
        std::map<std::string, std::string> entries;

        for (const auto& fs : filesystems)
        {
            auto v = fs->getFileNamesRecursive(directory, extension);
            for (const auto& filename : v)
            {
                entries.insert({filename, fs->getPath()});
            }
        }

        std::vector<std::pair<std::string, std::string>> v(entries.begin(), entries.end());
        return v;
    }

    void CompositeVirtualFileSystem::clear()
    {
        filesystems.clear();
    }

    void addHpisWithExtension(CompositeVirtualFileSystem& vfs, const fs::path& searchPath, const std::string& extension)
    {
        fs::directory_iterator it(searchPath);
        fs::directory_iterator end;

        for (; it != end; ++it)
        {
            const auto& e = *it;
            auto ext = e.path().extension().string();
            if (toUpper(ext) == toUpper(extension))
            {
                vfs.emplaceFileSystem<HpiFileSystem>(e.path().string());
            }
        }
    }

    void addToVfs(CompositeVirtualFileSystem& vfs, const boost::filesystem::path& searchPath)
    {
        std::vector<std::string> hpiExtensions{".hpi", ".ufo", ".ccx", ".gpf", ".gp3"};

        vfs.emplaceFileSystem<DirectoryFileSystem>(searchPath);

        // scan for HPIs to add
        for (auto it = hpiExtensions.rbegin(); it != hpiExtensions.rend(); ++it)
        {
            addHpisWithExtension(vfs, searchPath, *it);
        }
    }

    CompositeVirtualFileSystem constructVfs(const boost::filesystem::path& searchPath)
    {
        CompositeVirtualFileSystem vfs;
        addToVfs(vfs, searchPath);
        return vfs;
    }
}
