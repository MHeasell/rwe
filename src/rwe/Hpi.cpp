#include "Hpi.h"

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
        data = std::make_unique<char[]>(h.directorySize);
        start = h.start;
        readAndDecrypt(*stream, decryptionKey, data.get() + h.start, h.directorySize - h.start);
    }

    const HpiDirectoryEntry* HpiArchive::begin() const
    {
        auto directory = reinterpret_cast<HpiDirectoryData*>(data.get() + start);
        return reinterpret_cast<HpiDirectoryEntry*>(data.get() + directory->entryListOffset);
    }

    const HpiDirectoryEntry* HpiArchive::end() const
    {
        auto directory = reinterpret_cast<HpiDirectoryData*>(data.get() + start);
        return reinterpret_cast<HpiDirectoryEntry*>(data.get() + directory->entryListOffset) + directory->numberOfEntries;
    }
}
