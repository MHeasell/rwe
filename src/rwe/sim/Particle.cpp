#include "Particle.h"
#include <rwe/match.h>

namespace rwe
{
    bool Particle::isStarted(GameTime currentTime) const
    {
        return currentTime >= startTime;
    }

    unsigned int Particle::getFrameIndex(GameTime currentTime, GameTime frameDuration, int totalFrames) const
    {
        assert(currentTime >= startTime);
        auto deltaTime = currentTime - startTime;
        auto frameIndex = (deltaTime.value / frameDuration.value) % totalFrames;
        return frameIndex;
    }

    bool Particle::isFinished(GameTime currentTime, const ParticleFinishTime& finishTime, GameTime frameDuration, int numberOfFrames) const
    {
        auto concreteFinishTime = match(
            finishTime,
            [&](const ParticleFinishTimeFixedTime& t) { return t.time; },
            [&](const ParticleFinishTimeEndOfFrames&) { return startTime + (GameTime(numberOfFrames * frameDuration.value)); });

        return currentTime >= concreteFinishTime;
    }
}
