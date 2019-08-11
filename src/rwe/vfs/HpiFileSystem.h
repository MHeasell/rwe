#pragma once

#include <fstream>
#include <rwe/Hpi.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

namespace rwe
{
    class HpiFileSystem final : public AbstractVirtualFileSystem
    {
    private:
        class HpiRecursiveFilenamesVisitor
        {
        private:
            HpiFileSystem* fs;
            const std::string* name;
            const std::string* extension;

        public:
            HpiRecursiveFilenamesVisitor(HpiFileSystem* fs, const std::string* name, const std::string* extension);

            std::vector<std::string> operator()(const HpiArchive::File& e) const;

            std::vector<std::string> operator()(const HpiArchive::Directory& e) const;
        };

    private:
        std::string name;
        std::ifstream stream;
        HpiArchive hpi;

    public:
        explicit HpiFileSystem(const std::string& file);
        std::optional<std::vector<char>> readFile(const std::string& filename) const override;

        std::vector<std::string> getFileNames(const std::string& directory, const std::string& extension) override;

        std::vector<std::string>
        getFileNamesRecursive(const std::string& directory, const std::string& extension) override;

    private:
        std::vector<std::string> getFileNamesInternal(const HpiArchive::Directory& directory, const std::string& extension);
        std::vector<std::string> getFileNamesRecursiveInternal(const HpiArchive::Directory& directory, const std::string& extension);
    };
}
