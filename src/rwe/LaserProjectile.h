#ifndef RWE_LASERPROJECTILE_H
#define RWE_LASERPROJECTILE_H

#include <rwe/AudioService.h>
#include <rwe/PlayerId.h>
#include <rwe/SpriteSeries.h>
#include <rwe/geometry/Line3f.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct LaserProjectile
    {
        PlayerId owner;

        Vector3f position;

        Vector3f origin;

        /** Velocity in game pixels/tick */
        Vector3f velocity;

        /** Duration in ticks */
        float duration;

        Vector3f color;
        Vector3f color2;

        std::optional<AudioService::SoundHandle> soundHit;
        std::optional<AudioService::SoundHandle> soundWater;

        std::optional<std::shared_ptr<SpriteSeries>> explosion;
        std::optional<std::shared_ptr<SpriteSeries>> waterExplosion;

        Vector3f getBackPosition() const;
    };
}

#endif
