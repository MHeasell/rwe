#pragma once
#include <rwe/ProjectileRenderType.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/ProjectilePhysicsType.h>
#include <rwe/sim/SimAngle.h>
#include <rwe/sim/SimScalar.h>
#include <string>

namespace rwe
{
    struct WeaponDefinition
    {
        ProjectilePhysicsType physicsType;

        SimScalar maxRange;

        SimScalar reloadTime;

        /** The number of shots in a burst. */
        int burst;

        /** The amount of time between shots in the same burst, in seconds. */
        SimScalar burstInterval;

        /** Maximum angle deviation of projectiles shot in a burst. */
        SimAngle sprayAngle;

        SimAngle tolerance;

        SimAngle pitchTolerance;

        /** Projectile velocity in pixels/tick. */
        SimScalar velocity;

        /** If true, the weapon only fires on command and does not auto-target. */
        bool commandFire;

        std::unordered_map<std::string, unsigned int> damage;

        SimScalar damageRadius;

        /** Number of ticks projectiles fired from this weapon live for */
        std::optional<GameTime> weaponTimer;

        /** The range in frames that projectile lifetime may randomly vary by. */
        std::optional<GameTime> randomDecay;

        /** If true, projectile does not explode when hitting the ground but instead continues travelling. */
        bool groundBounce;
    };
}
