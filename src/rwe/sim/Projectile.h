#pragma once

#include <rwe/AudioService.h>
#include <rwe/ProjectileRenderType.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/PlayerId.h>
#include <rwe/sim/ProjectilePhysicsType.h>
#include <rwe/sim/SimVector.h>
#include <variant>

namespace rwe
{
    struct Projectile
    {
        std::string weaponType;

        PlayerId owner;

        SimVector position;
        SimVector previousPosition;

        SimVector origin;

        /** Velocity in game pixels/tick */
        SimVector velocity;

        bool gravity;

        ProjectileRenderType renderType;

        /** If true, creates smoke on impact. */
        bool endSmoke;

        /**
         * If set, smoke is emitted from the projectile.
         * The set value indicates the delay in ticks between each emission.
         * A value of 0 indicates that smoke is emitted every tick,
         * a value of 1 is every other tick, etc.
         */
        std::optional<GameTime> smokeTrail;

        /** The last time the projectile emitted smoke. */
        GameTime lastSmoke;

        std::unordered_map<std::string, int> damage;

        std::optional<GameTime> dieOnFrame;

        SimScalar damageRadius;

        bool groundBounce;

        bool isDead{false};

        /** The game time at which the projectile was created. */
        GameTime createdAt;

        SimVector getBackPosition(SimScalar duration) const;

        SimVector getPreviousBackPosition(SimScalar duration) const;

        int getDamage(const std::string& unitType) const;
    };
}
