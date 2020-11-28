#pragma once

#include <cstdint>
#include <functional>
#include <istream>
#include <optional>
#include <rwe/io/hpi/hpi_headers.h>
#include <variant>
#include <vector>

namespace rwe
{
    class HpiArchive
    {
    public:
        struct DirectoryEntry;
        struct File
        {
            enum class CompressionScheme
            {
                None = 0,
                LZ77,
                ZLib
            };
            CompressionScheme compressionScheme;
            std::size_t offset;
            std::size_t size;
        };
        struct Directory
        {
            std::vector<DirectoryEntry> entries;
        };
        struct DirectoryEntry
        {
            std::string name;
            std::variant<File, Directory> data;
        };

    private:
        std::istream* stream;
        unsigned char decryptionKey;
        Directory _root;

    public:
        explicit HpiArchive(std::istream* stream);

        const Directory& root() const;

        std::optional<std::reference_wrapper<const File>> findFile(const std::string& path) const;

        std::optional<std::reference_wrapper<const Directory>> findDirectory(const std::string& path) const;

        void extract(const File& file, char* buffer) const;
    };
}
