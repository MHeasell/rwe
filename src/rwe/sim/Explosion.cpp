#include "Explosion.h"
#include <rwe/overloaded.h>

namespace rwe
{
    bool Explosion::isStarted(GameTime currentTime) const
    {
        return currentTime >= startTime;
    }

    unsigned int Explosion::getFrameIndex(GameTime currentTime, int totalFrames) const
    {
        assert(currentTime >= startTime);
        auto deltaTime = currentTime - startTime;
        auto frameIndex = (deltaTime.value / frameDuration.value) % totalFrames;
        return frameIndex;
    }

    bool Explosion::isFinished(GameTime currentTime, int numberOfFrames) const
    {
        auto concreteFinishTime = match(
            finishTime,
            [&](const ExplosionFinishTimeFixedTime& t) { return t.time; },
            [&](const ExplosionFinishTimeEndOfFrames&) { return startTime + (GameTime(numberOfFrames * frameDuration.value)); });

        return currentTime >= concreteFinishTime;
    }
}
