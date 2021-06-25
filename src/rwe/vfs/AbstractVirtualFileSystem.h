#pragma once

#include <optional>
#include <rwe/io/gui/gui.h>
#include <string>
#include <vector>

namespace rwe
{
    class AbstractVirtualFileSystem
    {
    public:
        virtual ~AbstractVirtualFileSystem() = default;
        virtual std::optional<std::vector<char>> readFile(const std::string& filename) const = 0;
        virtual std::vector<std::string> getFileNames(const std::string& directory, const std::string& extension) = 0;
        virtual std::vector<std::string> getDirectoryNames(const std::string& directory) = 0;
        virtual std::vector<std::string> getFileNamesRecursive(const std::string& directory, const std::string& extension) = 0;

        std::vector<char> readFileOrThrow(const std::string& filename) const;
        std::vector<GuiEntry> readGuiOrThrow(const std::string& filename) const;
    };

    class LeafVirtualFileSystem : public AbstractVirtualFileSystem
    {
    public:
        virtual const std::string& getPath() const = 0;
    };
}
