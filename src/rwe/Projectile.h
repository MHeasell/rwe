#pragma once

#include <rwe/AudioService.h>
#include <rwe/GameTime.h>
#include <rwe/PlayerId.h>
#include <rwe/ProjectilePhysicsType.h>
#include <rwe/ProjectileRenderType.h>
#include <rwe/ShaderMesh.h>
#include <rwe/SimVector.h>
#include <rwe/SpriteSeries.h>
#include <rwe/math/Vector3x.h>
#include <variant>

namespace rwe
{
    struct Projectile
    {
        PlayerId owner;

        SimVector position;

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

        std::optional<AudioService::SoundHandle> soundHit;
        std::optional<AudioService::SoundHandle> soundWater;

        std::optional<std::shared_ptr<SpriteSeries>> explosion;
        std::optional<std::shared_ptr<SpriteSeries>> waterExplosion;

        std::unordered_map<std::string, unsigned int> damage;

        SimScalar damageRadius;

        bool isDead{false};

        SimVector getBackPosition(const ProjectileRenderTypeLaser& laserRenderType) const;

        unsigned int getDamage(const std::string& unitType) const;
    };
}
