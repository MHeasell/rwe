#ifndef RWE_COMPOSITEVIRTUALFILESYSTEM_H
#define RWE_COMPOSITEVIRTUALFILESYSTEM_H

#include <boost/filesystem.hpp>
#include <memory>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

namespace rwe
{
    class CompositeVirtualFileSystem final : public AbstractVirtualFileSystem
    {
    private:
        std::vector<std::unique_ptr<AbstractVirtualFileSystem>> filesystems;

    public:
        std::optional<std::vector<char>> readFile(const std::string& filename) const override;

        std::vector<std::string> getFileNames(const std::string& directory, const std::string& extension) override;

        std::vector<std::string>
        getFileNamesRecursive(const std::string& directory, const std::string& extension) override;

        template <typename T, typename... Args>
        void emplaceFileSystem(Args&&... args)
        {
            filesystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        }
    };


    CompositeVirtualFileSystem constructVfs(const boost::filesystem::path& searchPath);
}

#endif
