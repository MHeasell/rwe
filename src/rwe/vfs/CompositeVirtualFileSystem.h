#ifndef RWE_COMPOSITEVIRTUALFILESYSTEM_H
#define RWE_COMPOSITEVIRTUALFILESYSTEM_H

#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <memory>

namespace rwe
{
    class CompositeVirtualFileSystem final : public AbstractVirtualFileSystem
    {
    public:
        boost::optional<std::vector<char>> readFile(const std::string& filename) const override;

        std::vector<std::string> getFileNames(const std::string& directory, const std::string& extension) override;

        template <typename T, typename... Args>
        void emplaceFileSystem(Args&&... args)
        {
            filesystems.emplace_back(new T(std::forward<Args>(args)...));
        }

    private:
        std::vector<std::unique_ptr<AbstractVirtualFileSystem>> filesystems;
    };


    CompositeVirtualFileSystem constructVfs(const std::string& searchPath);
}

#endif
