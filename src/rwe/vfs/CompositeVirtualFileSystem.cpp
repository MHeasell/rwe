#include "CompositeVirtualFileSystem.h"
#include <boost/filesystem.hpp>
#include "HpiFileSystem.h"
#include <rwe/vfs/DirectoryFileSystem.h>
#include <rwe/vfs/HpiFileSystem.h>

#include <rwe/rwe_string.h>

namespace fs = boost::filesystem;

namespace rwe
{
    boost::optional<std::vector<char>> CompositeVirtualFileSystem::readFile(const std::string& filename) const
    {
        for (const auto& fs : filesystems)
        {
            auto file = fs->readFile(filename);
            if (file)
            {
                return file;
            }
        }

        return boost::none;
    }

    void addHpisWithExtension(rwe::CompositeVirtualFileSystem& vfs, const fs::path& searchPath, const std::string& extension)
    {
        for (const fs::directory_entry& e : fs::directory_iterator(searchPath))
        {
            auto ext = e.path().extension().string();
            if (toUpper(ext) == toUpper(extension))
            {
                vfs.emplaceFileSystem<rwe::HpiFileSystem>(e.path().string());
            }
        }
    }

    CompositeVirtualFileSystem* constructVfs(const std::string& searchPath)
    {
        std::vector<std::string> hpiExtensions{".hpi", ".ufo", ".ccx", ".gpf", ".gp3"};

        auto vfs = new rwe::CompositeVirtualFileSystem();
        vfs->emplaceFileSystem<rwe::DirectoryFileSystem>(searchPath);

        // scan for HPIs to add
        fs::path searchPathP(searchPath);
        for (auto it = --hpiExtensions.end(); it != hpiExtensions.begin(); --it)
        {
            addHpisWithExtension(*vfs, searchPathP, *it);
        }

        return vfs;
    }
}
