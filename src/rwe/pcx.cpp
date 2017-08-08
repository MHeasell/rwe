#include "pcx.h"

namespace rwe
{
    PcxException::PcxException(const char* message) : runtime_error(message) {}

    PcxDecoder::PcxDecoder(const char* begin, const char* end) : begin(begin), end(end)
    {
        header = reinterpret_cast<const PcxHeader*>(begin);
    }
}
