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

        /** The last time the projectile emitted smoke. */
        GameTime lastSmoke;

        std::unordered_map<std::string, unsigned int> damage;

        std::optional<GameTime> dieOnFrame;

        SimScalar damageRadius;

        bool groundBounce;

        bool isDead{false};

        /** The game time at which the projectile was created. */
        GameTime createdAt;

        SimVector getBackPosition(SimScalar duration) const;

        SimVector getPreviousBackPosition(SimScalar duration) const;

        unsigned int getDamage(const std::string& unitType) const;
    };
}
