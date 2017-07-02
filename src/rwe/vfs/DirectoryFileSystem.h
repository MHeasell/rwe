#ifndef RWE_DIRECTORYFILESYSTEM_H
#define RWE_DIRECTORYFILESYSTEM_H

#include <rwe/vfs/AbstractVirtualFileSystem.h>

#include <boost/filesystem.hpp>

namespace rwe
{
    class DirectoryFileSystem final : public AbstractVirtualFileSystem
    {
    public:
        explicit DirectoryFileSystem(const std::string& path);

        boost::optional<std::vector<char>> readFile(const std::string& filename) const override;

    private:
        boost::filesystem::path path;
    };
}

#endif
