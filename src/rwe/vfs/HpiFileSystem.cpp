#include "HpiFileSystem.h"
#include <rwe/rwe_string.h>

namespace rwe
{
    std::optional<std::vector<char>> HpiFileSystem::readFile(const std::string& filename) const
    {
        auto file = hpi.findFile(filename);
        if (!file)
        {
            return std::nullopt;
        }

        std::vector<char> buffer(file->get().size);
        hpi.extract(*file, buffer.data());

        return buffer;
    }

    HpiFileSystem::HpiFileSystem(const std::string& file)
        : name(file),
          stream(file, std::ios::binary),
          hpi(&stream)
    {
        if (!stream.is_open())
        {
            throw std::runtime_error("Could not open file");
        }
    }

    std::vector<std::string> HpiFileSystem::getFileNames(const std::string& directory, const std::string& extension)
    {
        auto dir = hpi.findDirectory(directory);
        if (!dir)
        {
            // no such directory, so no files inside it
            return std::vector<std::string>();
        }

        return getFileNamesInternal(*dir, extension);
    }

    std::vector<std::string>
    HpiFileSystem::getFileNamesRecursive(const std::string& directory, const std::string& extension)
    {
        auto dir = hpi.findDirectory(directory);
        if (!dir)
        {
            // no such directory, so no files inside it
            return std::vector<std::string>();
        }

        return getFileNamesRecursiveInternal(*dir, extension);
    }

    std::vector<std::string>
    HpiFileSystem::getFileNamesInternal(const HpiArchive::Directory& directory, const std::string& extension)
    {
        std::vector<std::string> v;

        for (const auto& e : directory.entries)
        {
            if (endsWith(toUpper(e.name), toUpper(extension)))
            {
                v.push_back(e.name);
            }
        }

        return v;
    }

    std::vector<std::string>
    HpiFileSystem::getFileNamesRecursiveInternal(const HpiArchive::Directory& directory, const std::string& extension)
    {
        std::vector<std::string> v;

        for (const auto& e : directory.entries)
        {
            auto innerEntries = boost::apply_visitor(HpiRecursiveFilenamesVisitor(this, &e.name, &extension), e.data);
            std::move(innerEntries.begin(), innerEntries.end(), std::back_inserter(v));
        }

        return v;
    }

    HpiFileSystem::HpiRecursiveFilenamesVisitor::HpiRecursiveFilenamesVisitor(
        HpiFileSystem* fs,
        const std::string* name,
        const std::string* extension)
        : fs(fs),
          name(name),
          extension(extension)
    {
    }

    std::vector<std::string> HpiFileSystem::HpiRecursiveFilenamesVisitor::operator()(const HpiArchive::File& /*e*/) const
    {
        return endsWith(toUpper(*name), toUpper(*extension)) ? std::vector<std::string>{*name} : std::vector<std::string>();
    }

    std::vector<std::string>
    HpiFileSystem::HpiRecursiveFilenamesVisitor::operator()(const HpiArchive::Directory& e) const
    {
        auto entries = fs->getFileNamesRecursiveInternal(e, *extension);
        std::vector<std::string> v;
        v.reserve(entries.size());
        for (const auto& entryName : entries)
        {
            v.push_back(*name + "/" + entryName);
        }
        return v;
    }
}
