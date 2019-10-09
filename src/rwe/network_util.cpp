#include "network_util.h"
#include <boost/crc.hpp>

namespace rwe
{
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
        // throw in a CRC to verify the message
        boost::crc_32_type crc;
        crc.process_bytes(buffer, size);
        return crc.checksum();
    }

}
