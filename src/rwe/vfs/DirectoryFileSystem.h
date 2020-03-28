#pragma once

#include <rwe/vfs/AbstractVirtualFileSystem.h>

#include <boost/filesystem.hpp>

namespace rwe
{
    class DirectoryFileSystem final : public LeafVirtualFileSystem
    {
    private:
        boost::filesystem::path path;
        std::string pathString;

    public:
        explicit DirectoryFileSystem(const std::string& path);
        explicit DirectoryFileSystem(const boost::filesystem::path& path);

    public:
        const std::string& getPath() const override;

        std::optional<std::vector<char>> readFile(const std::string& filename) const override;

        std::vector<std::string> getFileNames(const std::string& directory, const std::string& filter) override;

        std::vector<std::string> getFileNamesRecursive(const std::string& directory, const std::string& extension) override;
    };
}
