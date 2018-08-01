#include "Hpi.h"

#include <memory>
#include <optional>
#include <rwe/rwe_string.h>

#include <zlib.h>

namespace rwe
{
    HpiException::HpiException(const char* message) : runtime_error(message) {}

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
        if (key == 0)
        {
            return;
        }

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

    template <typename T>
    T readRaw(std::istream& stream)
    {
        T val;
        stream.read(reinterpret_cast<char*>(&val), sizeof(T));
        return val;
    }

    template <typename T>
    T readAndDecryptRaw(std::istream& stream, unsigned char key)
    {
        T val;
        readAndDecrypt(stream, key, reinterpret_cast<char*>(&val), sizeof(T));
        return val;
    }

    template <typename T>
    void readAndDecryptRawArray(std::istream& stream, unsigned char key, T* buffer, std::size_t size)
    {
        readAndDecrypt(stream, key, reinterpret_cast<char*>(buffer), sizeof(T) * size);
    }

    void decryptInner(char* buffer, std::size_t size)
    {
        for (std::size_t i = 0; i < size; ++i)
        {
            auto pos = static_cast<unsigned char>(i);
            buffer[i] = (buffer[i] - pos) ^ pos;
        }
    }

    uint32_t computeChecksum(const char* buffer, std::size_t size)
    {
        uint32_t sum = 0;
        for (std::size_t i = 0; i < size; ++i)
        {
            sum += static_cast<unsigned char>(buffer[i]);
        }

        return sum;
    }

    void decompressLZ77(const char* in, std::size_t len, char* out, std::size_t maxBytes)
    {
        char window[4096];

        std::size_t inPos = 0;
        std::size_t outPos = 0;
        unsigned int windowPos = 1;

        while (true)
        {
            if (inPos >= len)
            {
                throw HpiException("LZ77 decompress expected tag but got end of input");
            }

            auto tag = static_cast<unsigned char>(in[inPos++]);

            for (int i = 0; i < 8; ++i)
            {
                if ((tag & 1) == 0) // next byte is a literal byte
                {
                    if (inPos >= len)
                    {
                        throw HpiException("LZ77 decompress expected byte but got end of input");
                    }

                    if (outPos >= maxBytes)
                    {
                        throw HpiException("LZ77 decompress ran over max output bytes");
                    }

                    out[outPos++] = in[inPos];
                    window[windowPos] = in[inPos];
                    windowPos = (windowPos + 1) & 0xFFF;
                    inPos++;
                }
                else // next bytes point into the sliding window
                {
                    if (inPos >= len - 1)
                    {
                        throw HpiException("LZ77 decompress expected window offset/length but got end of input");
                    }

                    unsigned int packedData = *(reinterpret_cast<const uint16_t*>(&in[inPos]));
                    inPos += 2;

                    unsigned int offset = packedData >> 4;

                    if (offset == 0)
                    {
                        return;
                    }

                    unsigned int count = (packedData & 0x0F) + 2;

                    if (outPos + count > maxBytes)
                    {
                        throw HpiException("LZ77 decompress ran over max output bytes");
                    }

                    for (unsigned int x = 0; x < count; ++x)
                    {
                        out[outPos++] = window[offset];
                        window[windowPos] = window[offset];
                        offset = (offset + 1) & 0xFFF;
                        windowPos = (windowPos + 1) & 0xFFF;
                    }
                }

                tag >>= 1;
            }
        }
    }

    void decompressZLib(const char* in, std::size_t len, char* out, std::size_t maxBytes)
    {
        z_stream stream;

        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = 0;
        stream.next_in = Z_NULL;

        if (inflateInit(&stream) != Z_OK)
        {
            throw HpiException("ZLib decompress initialization failed");
        }

        stream.avail_in = static_cast<uInt>(len);
        stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(in));
        stream.avail_out = static_cast<uInt>(maxBytes);
        stream.next_out = reinterpret_cast<unsigned char*>(out);

        if (inflate(&stream, Z_NO_FLUSH) != Z_STREAM_END)
        {
            inflateEnd(&stream);
            throw HpiException("ZLib decompress failed");
        }

        inflateEnd(&stream);
    }

    std::optional<std::size_t> stringSize(const char* begin, const char* end)
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

        return std::nullopt;
    }

    HpiArchive::DirectoryEntry
    HpiArchive::convertDirectoryEntry(const HpiDirectoryEntry& entry, const char* buffer, std::size_t size)
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
            return DirectoryEntry{name, data};
        }
        else
        {
            if (entry.dataOffset + sizeof(HpiFileData) > size)
            {
                throw HpiException("Runaway file data offset");
            }

            auto f = reinterpret_cast<const HpiFileData*>(buffer + entry.dataOffset);
            auto data = convertFile(*f);
            return DirectoryEntry{name, data};
        }
    }

    HpiArchive::File HpiArchive::convertFile(const HpiFileData& file)
    {
        File f{static_cast<File::CompressionScheme>(file.compressionScheme), file.dataOffset, file.fileSize};
        return f;
    }

    HpiArchive::Directory
    HpiArchive::convertDirectory(const HpiDirectoryData& directory, const char* buffer, std::size_t size)
    {
        if (directory.entryListOffset + (directory.numberOfEntries * sizeof(HpiDirectoryEntry)) > size)
        {
            throw HpiException("Runaway directory entry list");
        }

        std::vector<DirectoryEntry> v;
        auto p = reinterpret_cast<const HpiDirectoryEntry*>(buffer + directory.entryListOffset);
        for (std::size_t i = 0; i < directory.numberOfEntries; ++i)
        {
            v.push_back(convertDirectoryEntry(p[i], buffer, size));
        }

        return Directory{v};
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
        switch (file.compressionScheme)
        {
            case HpiArchive::File::CompressionScheme::None:
                stream->seekg(file.offset);
                readAndDecrypt(*stream, decryptionKey, buffer, file.size);
                break;
            case HpiArchive::File::CompressionScheme::LZ77:
            case HpiArchive::File::CompressionScheme::ZLib:
                extractCompressed(file, buffer);
                break;
            default:
                throw HpiException("Invalid file entry compression scheme");
        }
    }

    void HpiArchive::extractCompressed(const HpiArchive::File& file, char* buffer) const
    {
        auto chunkCount = (file.size / 65536) + (file.size % 65536 == 0 ? 0 : 1);
        stream->seekg(file.offset);

        auto chunkSizes = std::make_unique<uint32_t[]>(chunkCount);
        readAndDecryptRawArray(*stream, decryptionKey, chunkSizes.get(), chunkCount);

        std::size_t bufferOffset = 0;
        for (std::size_t i = 0; i < chunkCount; ++i)
        {
            auto chunkHeader = readAndDecryptRaw<HpiChunk>(*stream, decryptionKey);
            if (chunkHeader.marker != HpiChunkMagicNumber)
            {
                throw HpiException("Invalid chunk header");
            }

            if (bufferOffset + chunkHeader.decompressedSize > file.size)
            {
                throw HpiException("Extracted file larger than expected");
            }

            auto chunkBuffer = std::make_unique<char[]>(chunkHeader.compressedSize);
            readAndDecrypt(*stream, decryptionKey, chunkBuffer.get(), chunkHeader.compressedSize);

            auto checksum = computeChecksum(chunkBuffer.get(), chunkHeader.compressedSize);
            if (checksum != chunkHeader.checksum)
            {
                throw HpiException("Invalid chunk checksum");
            }

            if (chunkHeader.encrypted != 0)
            {
                decryptInner(chunkBuffer.get(), chunkHeader.compressedSize);
            }

            switch (chunkHeader.compressionScheme)
            {
                case 0: // no compression
                    if (chunkHeader.compressedSize != chunkHeader.decompressedSize)
                    {
                        throw HpiException("Uncompressed chunk has different decompressed and compressed sizes");
                    }

                    std::copy(chunkBuffer.get(), chunkBuffer.get() + chunkHeader.compressedSize, buffer + bufferOffset);
                    bufferOffset += chunkHeader.decompressedSize;
                    break;

                case 1: // LZ77 compression
                    decompressLZ77(chunkBuffer.get(), chunkHeader.compressedSize, buffer + bufferOffset, chunkHeader.decompressedSize);
                    bufferOffset += chunkHeader.decompressedSize;
                    break;

                case 2: // ZLib compression
                    decompressZLib(chunkBuffer.get(), chunkHeader.compressedSize, buffer + bufferOffset, chunkHeader.decompressedSize);
                    bufferOffset += chunkHeader.decompressedSize;
                    break;
                default:
                    throw HpiException("Invalid compression scheme");
            }
        }
    }

    struct FileToOptionalVisitor : public boost::static_visitor<std::optional<std::reference_wrapper<const HpiArchive::File>>>
    {
        std::optional<std::reference_wrapper<const HpiArchive::File>> operator()(const HpiArchive::File& f) const
        {
            return f;
        }

        std::optional<std::reference_wrapper<const HpiArchive::File>> operator()(const HpiArchive::Directory& /*d*/) const
        {
            return std::nullopt;
        }
    };

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

        return boost::apply_visitor(FileToOptionalVisitor(), it->data);
    }

    struct DirToOptionalVisitor : public boost::static_visitor<std::optional<std::reference_wrapper<const HpiArchive::Directory>>>
    {
        std::optional<std::reference_wrapper<const HpiArchive::Directory>> operator()(const HpiArchive::File& /*f*/) const
        {
            return std::nullopt;
        }

        std::optional<std::reference_wrapper<const HpiArchive::Directory>> operator()(const HpiArchive::Directory& d) const
        {
            return d;
        }
    };

    std::optional<std::reference_wrapper<const HpiArchive::File>> HpiArchive::findFile(const std::string& path) const
    {
        auto components = split(path, {'/'});

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

            auto foundDir = boost::apply_visitor(DirToOptionalVisitor(), it->data);
            if (!foundDir)
            {
                return std::nullopt;
            }

            dir = &foundDir->get();
        }

        // find the file in the directory
        return findFileInner(*dir, components.back());
    }

    std::optional<std::reference_wrapper<const HpiArchive::Directory>> HpiArchive::findDirectory(const std::string& path) const
    {
        auto components = split(path, {'/'});

        const Directory* dir = &root();

        // traverse to the correct directory
        for (auto cIt = components.cbegin(), cEnd = components.cend(); cIt != cEnd; ++cIt)
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

            auto foundDir = boost::apply_visitor(DirToOptionalVisitor(), it->data);
            if (!foundDir)
            {
                return std::nullopt;
            }

            dir = &foundDir->get();
        }

        return *dir;
    }
}
