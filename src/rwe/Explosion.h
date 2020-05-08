#pragma once

#include <memory>
#include <rwe/GameTime.h>
#include <rwe/SimVector.h>
#include <rwe/SpriteSeries.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct Explosion
    {
        SimVector position;
        std::shared_ptr<SpriteSeries> animation;
        GameTime startTime;
        GameTime finishTime;
        GameTime frameDuration{4};

        /** If true, the particle moves upwards each tick, as smoke. */
        bool floats{false};
        bool translucent{false};

        bool isStarted(GameTime currentTime) const;
        unsigned int getFrameIndex(GameTime currentTime) const;
        bool isFinished(GameTime currentTime) const;
    };
}
