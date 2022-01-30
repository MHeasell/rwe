#pragma once

#include <rwe/math/Vector3f.h>
#include <rwe/sim/GameTime.h>
namespace rwe
{
    struct FlashEffect
    {
        Vector3f position;
        GameTime startTime;
        GameTime duration;
        float maxRadius;
        float maxIntensity;
        Vector3f color;

        std::optional<float> getFractionComplete(GameTime currentTime) const;

        bool isFinished(GameTime currentTime) const;
    };
}
