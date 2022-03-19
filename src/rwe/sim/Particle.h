#pragma once

#include <memory>
#include <rwe/math/Vector3f.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/SimVector.h>
#include <variant>

namespace rwe
{
    struct ParticleFinishTimeEndOfFrames
    {
    };
    struct ParticleFinishTimeFixedTime
    {
        GameTime time;
    };
    using ParticleFinishTime = std::variant<ParticleFinishTimeEndOfFrames, ParticleFinishTimeFixedTime>;
    struct Particle
    {
        Vector3f position;
        std::string gafName;
        std::string animName;
        GameTime startTime;
        ParticleFinishTime finishTime;
        GameTime frameDuration{4};

        /** If true, the particle moves upwards each tick, as smoke. */
        bool floats{false};
        bool translucent{false};

        bool isStarted(GameTime currentTime) const;
        unsigned int getFrameIndex(GameTime currentTime, int totalFrames) const;
        bool isFinished(GameTime currentTime, int numberOfFrames) const;
    };
}
