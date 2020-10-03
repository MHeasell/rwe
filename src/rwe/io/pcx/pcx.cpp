#include "pcx.h"

namespace rwe
{
    PcxException::PcxException(const char* message) : runtime_error(message) {}
}
