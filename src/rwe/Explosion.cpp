#include "Explosion.h"

namespace rwe
{
    bool Explosion::isStarted(GameTime currentTime) const
    {
        return currentTime >= startTime;
    }

    unsigned int Explosion::getFrameIndex(GameTime currentTime) const
    {
        assert(currentTime >= startTime);
        auto deltaTime = currentTime - startTime;
        auto frameIndex = (deltaTime.value / frameDuration.value) % animation->sprites.size();
        return frameIndex;
    }

    bool Explosion::isFinished(GameTime currentTime) const
    {
        return currentTime >= finishTime;
    }
}
