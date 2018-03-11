#ifndef RWE_VIRTUALFILESYSTEM_H
#define RWE_VIRTUALFILESYSTEM_H

#include <optional>
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
        virtual std::vector<std::string> getFileNamesRecursive(const std::string& directory, const std::string& extension) = 0;
    };
}

#endif
