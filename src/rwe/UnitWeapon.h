#ifndef RWE_UNITWEAPON_H
#define RWE_UNITWEAPON_H

#include "GameTime.h"
#include <boost/variant.hpp>
#include <rwe/cob/CobThread.h>

namespace rwe
{
    struct UnitWeaponStateIdle
    {
    };
    struct UnitWeaponStateAiming
    {
        const CobThread* aimingThread;
    };
    struct UnitWeaponStateReloading
    {
        /** The game time at which the weapon next becomes ready to fire. */
        GameTime readyTime;
    };
    using UnitWeaponState = boost::variant<UnitWeaponStateIdle, UnitWeaponStateAiming, UnitWeaponStateReloading>;

    struct UnitWeapon
    {
        UnitWeaponState state{UnitWeaponStateIdle()};
    };
}

#endif
