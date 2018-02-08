#ifndef RWE_UNITWEAPON_H
#define RWE_UNITWEAPON_H

#include "GameTime.h"

namespace rwe
{
    struct UnitWeapon
    {
        /** The game time at which the weapon next becomes ready to fire. */
        GameTime readyTime{0};

        bool isAiming{false};
    };
}

#endif
