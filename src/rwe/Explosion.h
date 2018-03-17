#ifndef RWE_EXPLOSION_H
#define RWE_EXPLOSION_H

#include <memory>
#include <rwe/GameTime.h>
#include <rwe/SpriteSeries.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct Explosion
    {
        Vector3f position;
        std::shared_ptr<SpriteSeries> animation;
        GameTime startTime;

        bool isStarted(GameTime currentTime) const;
        unsigned int getFrameIndex(GameTime currentTime) const;
        bool isFinished(GameTime currentTime) const;
    };
}

#endif
