#include "Hpi.h"

#include <boost/optional.hpp>

namespace rwe
{
    HpiException::HpiException(const char* message) : runtime_error(message) {}

    template<typename T>
    T readRaw(std::istream& stream)
    {
        T val;
        stream.read(reinterpret_cast<char*>(&val), sizeof(T));
        return val;
    }

    /**
     * Decrypts the contents of the buffer.
     * @param key The decryption key.
     * @param seed The seed for decryption,
     * typically the position of the starting byte in the file.
     * @param buf The buffer to decrypt.
     * @param size The size of the buffer.
     */
    void decrypt(unsigned char key, unsigned char seed, char buf[], std::streamsize size)
    {
        for (std::streamsize i = 0; i < size; ++i)
        {
            auto pos = seed + static_cast<unsigned char>(i);
            buf[i] = (pos ^ key) ^ buf[i];
        }
    }

    void readAndDecrypt(std::istream& stream, unsigned char key, char buf[], std::streamsize size)
    {
        auto seed = static_cast<unsigned char>(stream.tellg());
        stream.read(buf, size);
        decrypt(key, seed, buf, stream.gcount());
    }

    unsigned char transformKey(unsigned char key)
    {
        return (key << 2) | (key >> 6);
    }

    boost::optional<std::size_t> stringSize(const char* begin, const char* end)
    {
        std::size_t count = 0;
        while (begin != end)
        {
            if (*begin == '\0')
            {
                return count;
            }

            ++count;
            ++begin;
        }

        return boost::none;
    }

    HpiArchive::DirectoryEntry HpiArchive::convertDirectoryEntry(const HpiDirectoryEntry& entry, const char* buffer, std::size_t size)
    {
        auto nameSize = stringSize(buffer + entry.nameOffset, buffer + size);
        if (!nameSize)
        {
            throw std::runtime_error("Invalid HPI file, runaway directory entry name");
        }

        std::string name(buffer + entry.nameOffset, *nameSize);
        if (entry.isDirectory != 0)
        {
            assert(entry.dataOffset + sizeof(HpiDirectoryData) <= size);
            auto d = reinterpret_cast<const HpiDirectoryData*>(buffer + entry.dataOffset);
            auto data = convertDirectory(*d, buffer, size);
            return DirectoryEntry { name, data };
        }
        else
        {
            assert(entry.dataOffset + sizeof(HpiFileData) <= size);
            auto f = reinterpret_cast<const HpiFileData*>(buffer + entry.dataOffset);
            auto data = convertFile(*f, buffer, size);
            return DirectoryEntry { name, data };
        }
    }

    HpiArchive::File HpiArchive::convertFile(const HpiFileData& file, const char* buffer, std::size_t size)
    {
        File f { static_cast<File::CompressionScheme>(file.compressionScheme), file.dataOffset, file.fileSize};
        return f;
    }

    HpiArchive::Directory HpiArchive::convertDirectory(const HpiDirectoryData& directory, const char* buffer, std::size_t size)
    {
        std::vector<DirectoryEntry> v;
        auto end = directory.entryListOffset + (directory.numberOfEntries * sizeof(HpiDirectoryEntry));
        assert(end <= size);
        auto p = reinterpret_cast<const HpiDirectoryEntry*>(buffer + directory.entryListOffset);
        for (std::size_t i = 0; i < directory.numberOfEntries; ++i)
        {
            v.push_back(convertDirectoryEntry(p[i], buffer, size));
        }

        return Directory { v };
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

        assert(h.directorySize - h.start >= sizeof(HpiDirectoryData));
        auto directory = reinterpret_cast<HpiDirectoryData*>(data.get() + h.start);
        _root = convertDirectory(*directory, data.get(), h.directorySize);
    }

    const HpiArchive::Directory HpiArchive::root() const
    {
        return _root;
    }
}
