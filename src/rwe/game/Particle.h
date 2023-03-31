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

    struct ParticleRenderTypeSprite
    {
        std::string gafName;
        std::string animName;
        ParticleFinishTime finishTime;
        GameTime frameDuration{4};
        bool translucent{false};
    };

    struct ParticleRenderTypeWake
    {
        GameTime finishTime;
    };

    using ParticleRenderType = std::variant<ParticleRenderTypeSprite, ParticleRenderTypeWake>;

    struct Particle
    {
        Vector3f position;
        Vector3f velocity;
        ParticleRenderType renderType;
        GameTime startTime;

        bool isStarted(GameTime currentTime) const;
        unsigned int getFrameIndex(GameTime currentTime, GameTime frameDuration, int totalFrames) const;
        bool isFinished(GameTime currentTime, const ParticleFinishTime& finishTime, GameTime frameDuration, int numberOfFrames) const;
    };
}
