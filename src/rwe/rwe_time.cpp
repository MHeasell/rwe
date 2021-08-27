#include "rwe_time.h"

namespace rwe
{
    Timestamp getTimestamp()
    {
        return std::chrono::steady_clock::now();
    }

    int TimeService::getTicks() // TODO (kwh) - should be getElapsed_ms? Int is sufficient, won't roll over until after ~7 days
    {
        return static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(getTimestamp() - initTime).count());
    }
}
