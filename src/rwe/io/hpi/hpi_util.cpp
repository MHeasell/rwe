#include "hpi_util.h"

#include <memory>
#include <rwe/io/hpi/hpi_headers.h>
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

    void extractCompressed(std::istream& stream, unsigned char decryptionKey, char* buffer, std::size_t size)
    {
        auto chunkCount = (size / 65536) + (size % 65536 == 0 ? 0 : 1);

        auto chunkSizes = std::make_unique<uint32_t[]>(chunkCount);
        readAndDecryptRawArray(stream, decryptionKey, chunkSizes.get(), chunkCount);

        std::size_t bufferOffset = 0;
        for (std::size_t i = 0; i < chunkCount; ++i)
        {
            auto chunkHeader = readAndDecryptRaw<HpiChunk>(stream, decryptionKey);
            if (chunkHeader.marker != HpiChunkMagicNumber)
            {
                throw HpiException("Invalid chunk header");
            }

            if (bufferOffset + chunkHeader.decompressedSize > size)
            {
                throw HpiException("Extracted file larger than expected");
            }

            auto chunkBuffer = std::make_unique<char[]>(chunkHeader.compressedSize);
            readAndDecrypt(stream, decryptionKey, chunkBuffer.get(), chunkHeader.compressedSize);

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
}
