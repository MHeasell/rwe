#include "FlashEffect.h"

namespace rwe
{
    std::optional<float> FlashEffect::getFractionComplete(GameTime currentTime) const
    {
        if (currentTime < startTime)
        {
            return std::nullopt;
        }

        if (currentTime >= startTime + duration)
        {
            return std::nullopt;
        }

        auto elapsedTime = currentTime - startTime;

        return static_cast<float>(elapsedTime.value) / static_cast<float>(duration.value);
    }

    bool FlashEffect::isFinished(GameTime currentTime) const
    {
        return currentTime >= startTime + duration;
    }
}
