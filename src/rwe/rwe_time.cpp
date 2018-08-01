#include "rwe_time.h"

namespace rwe
{
    Timestamp getTimestamp()
    {
        return std::chrono::steady_clock::now();
    }

    unsigned int TimeService::getTicks()
    {
        return static_cast<unsigned int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(getTimestamp() - initTime).count());
    }
}
