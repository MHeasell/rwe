#pragma once
#include <istream>
#include <optional>

namespace rwe
{
    class HpiException : public std::runtime_error
    {
    public:
        explicit HpiException(const char* message);
    };

    void decrypt(unsigned char key, unsigned char seed, char buf[], std::streamsize size);

    void readAndDecrypt(std::istream& stream, unsigned char key, char buf[], std::streamsize size);

    unsigned char transformKey(unsigned char key);

    void decryptInner(char* buffer, std::size_t size);

    uint32_t computeChecksum(const char* buffer, std::size_t size);

    void decompressLZ77(const char* in, std::size_t len, char* out, std::size_t maxBytes);

    void decompressZLib(const char* in, std::size_t len, char* out, std::size_t maxBytes);

    std::optional<std::size_t> stringSize(const char* begin, const char* end);

    void extractCompressed(std::istream& stream, unsigned char decryptionKey, char* buffer, std::size_t size);

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
}
