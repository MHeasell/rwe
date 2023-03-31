#include "HpiArchive.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <rwe/io/hpi/hpi_util.h>
#include <rwe/io/io_util.h>
#include <rwe/util/match.h>
#include <rwe/util/rwe_string.h>

namespace rwe
{
    HpiArchive::DirectoryEntry
    convertDirectoryEntry(const HpiDirectoryEntry& entry, const char* buffer, std::size_t size);

    HpiArchive::File convertFile(const HpiFileData& file)
    {
        HpiArchive::File f{static_cast<HpiArchive::File::CompressionScheme>(file.compressionScheme), file.dataOffset, file.fileSize};
        return f;
    }

    HpiArchive::Directory
    convertDirectory(const HpiDirectoryData& directory, const char* buffer, std::size_t size)
    {
        if (directory.entryListOffset + (directory.numberOfEntries * sizeof(HpiDirectoryEntry)) > size)
        {
            throw HpiException("Runaway directory entry list");
        }

        std::vector<HpiArchive::DirectoryEntry> v;
        auto p = reinterpret_cast<const HpiDirectoryEntry*>(buffer + directory.entryListOffset);
        for (std::size_t i = 0; i < directory.numberOfEntries; ++i)
        {
            v.push_back(convertDirectoryEntry(p[i], buffer, size));
        }

        return HpiArchive::Directory{v};
    }

    HpiArchive::DirectoryEntry
    convertDirectoryEntry(const HpiDirectoryEntry& entry, const char* buffer, std::size_t size)
    {
        auto nameSize = stringSize(buffer + entry.nameOffset, buffer + size);
        if (!nameSize)
        {
            throw HpiException("Runaway directory entry name");
        }

        std::string name(buffer + entry.nameOffset, *nameSize);
        if (entry.isDirectory != 0)
        {
            if (entry.dataOffset + sizeof(HpiDirectoryData) > size)
            {
                throw HpiException("Runaway directory data offset");
            }

            auto d = reinterpret_cast<const HpiDirectoryData*>(buffer + entry.dataOffset);
            auto data = convertDirectory(*d, buffer, size);
            return HpiArchive::DirectoryEntry{name, data};
        }
        else
        {
            if (entry.dataOffset + sizeof(HpiFileData) > size)
            {
                throw HpiException("Runaway file data offset");
            }

            auto f = reinterpret_cast<const HpiFileData*>(buffer + entry.dataOffset);
            auto data = convertFile(*f);
            return HpiArchive::DirectoryEntry{name, data};
        }
    }

    HpiArchive::HpiArchive(std::istream* stream) : stream(stream)
    {
        auto v = readRaw<HpiVersion>(*stream);
        if (v.marker != HpiMagicNumber)
        {
            throw HpiException("Invalid HPI file marker");
        }

        if (v.version != HpiVersionNumber)
        {
            throw HpiException("Unsupported HPI version");
        }

        auto h = readRaw<HpiHeader>(*stream);

        decryptionKey = transformKey(static_cast<unsigned char>(h.headerKey));

        stream->seekg(h.start);
        auto data = std::make_unique<char[]>(h.directorySize);
        readAndDecrypt(*stream, decryptionKey, data.get() + h.start, h.directorySize - h.start);

        if (h.start + sizeof(HpiDirectoryData) > h.directorySize)
        {
            throw HpiException("Runaway root directory");
        }

        auto directory = reinterpret_cast<HpiDirectoryData*>(data.get() + h.start);
        _root = convertDirectory(*directory, data.get(), h.directorySize);
    }

    const HpiArchive::Directory& HpiArchive::root() const
    {
        return _root;
    }

    void HpiArchive::extract(const HpiArchive::File& file, char* buffer) const
    {
        stream->seekg(file.offset);
        switch (file.compressionScheme)
        {
            case HpiArchive::File::CompressionScheme::None:
                readAndDecrypt(*stream, decryptionKey, buffer, file.size);
                break;
            case HpiArchive::File::CompressionScheme::LZ77:
            case HpiArchive::File::CompressionScheme::ZLib:
                extractCompressed(*stream, decryptionKey, buffer, file.size);
                break;
            default:
                throw HpiException("Invalid file entry compression scheme");
        }
    }

    std::optional<std::reference_wrapper<const HpiArchive::File>> findFileInner(const HpiArchive::Directory& dir, const std::string& name)
    {
        auto it = std::find_if(
            dir.entries.begin(),
            dir.entries.end(),
            [name](const HpiArchive::DirectoryEntry& e) {
                return toUpper(e.name) == toUpper(name);
            });

        if (it == dir.entries.end())
        {
            return std::nullopt;
        }

        return match(
            it->data,
            [&](const HpiArchive::Directory&) { return std::optional<std::reference_wrapper<const HpiArchive::File>>(); },
            [&](const HpiArchive::File& f) { return std::make_optional(std::ref(f)); });
    }

    std::optional<std::reference_wrapper<const HpiArchive::File>> HpiArchive::findFile(const std::string& path) const
    {
        auto components = split(path, '/');

        const Directory* dir = &root();

        // traverse to the correct directory
        for (auto cIt = components.cbegin(), cEnd = --components.cend(); cIt != cEnd; ++cIt)
        {
            auto& c = *cIt;
            auto begin = dir->entries.begin();
            auto end = dir->entries.end();
            auto it = std::find_if(
                begin,
                end,
                [c](const DirectoryEntry& e) {
                    return toUpper(e.name) == toUpper(c);
                });
            if (it == end)
            {
                return std::nullopt;
            }

            auto foundDir = std::get_if<Directory>(&it->data);
            if (!foundDir)
            {
                return std::nullopt;
            }

            dir = foundDir;
        }

        // find the file in the directory
        return findFileInner(*dir, components.back());
    }

    std::optional<std::reference_wrapper<const HpiArchive::Directory>> HpiArchive::findDirectory(const std::string& path) const
    {
        auto components = split(path, '/');

        const Directory* dir = &root();

        // traverse to the correct directory
        for (const auto& c : components)
        {
            auto begin = dir->entries.begin();
            auto end = dir->entries.end();
            auto it = std::find_if(
                begin,
                end,
                [c](const DirectoryEntry& e) {
                    return toUpper(e.name) == toUpper(c);
                });
            if (it == end)
            {
                return std::nullopt;
            }

            auto foundDir = std::get_if<Directory>(&it->data);
            if (!foundDir)
            {
                return std::nullopt;
            }

            dir = foundDir;
        }

        return *dir;
    }
}
