#include <rwe/gui.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

namespace rwe
{
    std::vector<char> AbstractVirtualFileSystem::readFileOrThrow(const std::string& filename) const
    {
        auto bytes = readFile(filename);
        if (!bytes)
        {
            throw std::runtime_error("Couldn't read " + filename);
        }
        return *bytes;
    }

    std::vector<GuiEntry> AbstractVirtualFileSystem::readGuiOrThrow(const std::string& filename) const
    {
        auto parsedGui = parseGuiFromBytes(readFileOrThrow(filename));
        if (!parsedGui)
        {
            throw std::runtime_error("Failed to parse GUI file: " + filename);
        }
        return *parsedGui;
    }
}
