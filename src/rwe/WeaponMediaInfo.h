#pragma once

#include <optional>
#include <rwe/ProjectileRenderType.h>
#include <rwe/sim/GameTime.h>
#include <string>

namespace rwe
{
    struct AnimLocation
    {
        std::string gafName;
        std::string animName;
    };

    struct WeaponMediaInfo
    {
        /** If true, creates smoke when fired. */
        bool startSmoke;

        /** If true, creates smoke on impact. */
        bool endSmoke;

        /**
         * If set, smoke is emitted from the projectile.
         * The set value indicates the delay in ticks between each emission.
         * A value of 0 indicates that smoke is emitted every tick,
         * a value of 1 is every other tick, etc.
         */
        std::optional<GameTime> smokeTrail;

        ProjectileRenderType renderType;

        /** If true, weapon start sound plays on each shot of the burst. Otherwise plays only on first shot. */
        bool soundTrigger;

        std::optional<std::string> soundStart;

        std::optional<std::string> soundHit;

        std::optional<std::string> soundWater;

        std::optional<AnimLocation> explosionAnim;

        std::optional<AnimLocation> waterExplosionAnim;
    };

}
