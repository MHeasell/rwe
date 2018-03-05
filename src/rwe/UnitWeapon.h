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
        /** The target the weapon is currently trying to shoot at. */
        UnitWeaponAttackTarget target;

        /** The unit's aiming thread. */
        boost::optional<const CobThread*> aimingThread;

        UnitWeaponStateAttacking(const UnitWeaponAttackTarget& target) : target(target)
        {
        }
    };

    using UnitWeaponState = boost::variant<UnitWeaponStateIdle, UnitWeaponStateAttacking>;

    struct UnitWeapon
    {
        float maxRange;

        float reloadTime;

        boost::optional<AudioService::SoundHandle> soundStart;
        boost::optional<AudioService::SoundHandle> soundHit;
        boost::optional<AudioService::SoundHandle> soundWater;

        /** The game time at which the weapon next becomes ready to fire. */
        GameTime readyTime{0};

        /** The internal state of the weapon. */
        UnitWeaponState state{UnitWeaponStateIdle()};
    };
}

#endif
