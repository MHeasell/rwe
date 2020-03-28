#pragma once

#include <boost/filesystem.hpp>
#include <memory>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

namespace rwe
{
    class CompositeVirtualFileSystem final : public AbstractVirtualFileSystem
    {
    private:
        std::vector<std::unique_ptr<LeafVirtualFileSystem>> filesystems;

    public:
        std::optional<std::vector<char>> readFile(const std::string& filename) const override;

        std::optional<std::vector<char>> readFileFromSource(const std::string& source, const std::string& filename) const;

        std::vector<std::string> getFileNames(const std::string& directory, const std::string& extension) override;

        std::vector<std::pair<std::string, std::string>> getFileNamesWithSources(const std::string& directory, const std::string& extension);

        std::vector<std::string>
        getFileNamesRecursive(const std::string& directory, const std::string& extension) override;

        std::vector<std::pair<std::string, std::string>>
        getFileNamesRecursiveWithSources(const std::string& directory, const std::string& extension);

        void clear();

        template <typename T, typename... Args>
        void emplaceFileSystem(Args&&... args)
        {
            filesystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        }
    };


    void addToVfs(CompositeVirtualFileSystem& vfs, const boost::filesystem::path& searchPath);
    CompositeVirtualFileSystem constructVfs(const boost::filesystem::path& searchPath);
}
