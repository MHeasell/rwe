#ifndef RWE_HPIFILESYSTEM_H
#define RWE_HPIFILESYSTEM_H

#include <fstream>
#include <rwe/Hpi.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

namespace rwe
{
    class HpiFileSystem final : public AbstractVirtualFileSystem
    {
    public:
        explicit HpiFileSystem(const std::string& file);
        boost::optional<std::vector<char>> readFile(const std::string& filename) const override;

        std::vector<std::string> getFileNames(const std::string& directory, const std::string& extension) override;

    private:
        std::ifstream stream;
        HpiArchive hpi;
    };
}

#endif
