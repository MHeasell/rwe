#ifndef RWE_UNITWEAPON_H
#define RWE_UNITWEAPON_H

#include <boost/variant.hpp>
#include <rwe/AudioService.h>
#include <rwe/GameTime.h>
#include <rwe/UnitId.h>
#include <rwe/cob/CobThread.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct UnitWeaponStateIdle
    {
    };

    using UnitWeaponAttackTarget = boost::variant<UnitId, Vector3f>;

    struct UnitWeaponStateAttacking
    {
        struct AimInfo
        {
            const CobThread* thread;
            float lastHeading;
            float lastPitch;
        };

        /** The target the weapon is currently trying to shoot at. */
        UnitWeaponAttackTarget target;

        std::optional<AimInfo> aimInfo;

        UnitWeaponStateAttacking(const UnitWeaponAttackTarget& target) : target(target)
        {
        }
    };

    using UnitWeaponState = boost::variant<UnitWeaponStateIdle, UnitWeaponStateAttacking>;

    struct UnitWeapon
    {
        float maxRange;

        float reloadTime;

        std::optional<AudioService::SoundHandle> soundStart;
        std::optional<AudioService::SoundHandle> soundHit;
        std::optional<AudioService::SoundHandle> soundWater;

        /** The game time at which the weapon next becomes ready to fire. */
        GameTime readyTime{0};

        float tolerance;

        float pitchTolerance;

        /** Projectile velocity in pixels/tick. */
        float velocity;

        /** Beam duration in ticks. */
        float duration;

        /** The internal state of the weapon. */
        UnitWeaponState state{UnitWeaponStateIdle()};
    };
}

#endif
