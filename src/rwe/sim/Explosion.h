#pragma once

#include <memory>
#include <rwe/math/Vector3f.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/SimVector.h>
#include <variant>

namespace rwe
{
    struct ExplosionFinishTimeEndOfFrames
    {
    };
    struct ExplosionFinishTimeFixedTime
    {
        GameTime time;
    };
    using ExplosionFinishTime = std::variant<ExplosionFinishTimeEndOfFrames, ExplosionFinishTimeFixedTime>;
    struct Explosion
    {
        Vector3f position;
        std::string explosionGaf;
        std::string explosionAnim;
        GameTime startTime;
        ExplosionFinishTime finishTime;
        GameTime frameDuration{4};

        /** If true, the particle moves upwards each tick, as smoke. */
        bool floats{false};
        bool translucent{false};

        bool isStarted(GameTime currentTime) const;
        int getFrameIndex(GameTime currentTime, int totalFrames) const;
        bool isFinished(GameTime currentTime, int numberOfFrames) const;
    };
}
