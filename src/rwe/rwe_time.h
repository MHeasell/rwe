#pragma once

#include <chrono>

namespace rwe
{
    using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;
    Timestamp getTimestamp();

    class TimeService
    {
    private:
        const Timestamp initTime;

    public:
        explicit TimeService(const Timestamp& initTime) : initTime(initTime)
        {
        }

        unsigned int getTicks();
    };
}
