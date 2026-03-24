#include "network_util.h"
#include <cstdint>

namespace rwe
{
    // CRC32 lookup table (polynomial 0xEDB88320, same as Boost.CRC / zlib)
    static uint32_t makeCrc32Entry(uint32_t index)
    {
        uint32_t crc = index;
        for (int j = 0; j < 8; ++j)
        {
            crc = (crc >> 1) ^ (0xEDB88320u & (-(crc & 1u)));
        }
        return crc;
    }

    struct Crc32Table
    {
        uint32_t entries[256];
        Crc32Table()
        {
            for (uint32_t i = 0; i < 256; ++i)
            {
                entries[i] = makeCrc32Entry(i);
            }
        }
    };

    static const Crc32Table crc32Table;

    float ema(float val, float average, float alpha)
    {
        return (alpha * val) + ((1.0f - alpha) * average);
    }
    void writeInt(char* sendBuffer, unsigned int crcResult)
    {
        sendBuffer[0] = crcResult & 0xffu;
        sendBuffer[1] = (crcResult >> 8u) & 0xffu;
        sendBuffer[2] = (crcResult >> 16u) & 0xffu;
        sendBuffer[3] = (crcResult >> 24u) & 0xffu;
    }
    unsigned int readInt(const char* buffer)
    {
        return (static_cast<unsigned int>(static_cast<unsigned char>(buffer[3])) << 24u)
            | (static_cast<unsigned int>(static_cast<unsigned char>(buffer[2])) << 16u)
            | (static_cast<unsigned int>(static_cast<unsigned char>(buffer[1])) << 8u)
            | (static_cast<unsigned int>(static_cast<unsigned char>(buffer[0])));
    }
    unsigned int computeCrc(const char* buffer, unsigned int size)
    {
        uint32_t crc = 0xFFFFFFFFu;
        for (unsigned int i = 0; i < size; ++i)
        {
            crc = crc32Table.entries[(crc ^ static_cast<unsigned char>(buffer[i])) & 0xFFu] ^ (crc >> 8u);
        }
        return crc ^ 0xFFFFFFFFu;
    }

}
