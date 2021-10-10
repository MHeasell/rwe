#pragma once

#include <rwe/AudioService.h>
#include <rwe/ProjectileRenderType.h>
#include <rwe/cob/CobThread.h>
#include <rwe/math/Vector3f.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/ProjectilePhysicsType.h>
#include <rwe/sim/UnitId.h>
#include <rwe/sim/WeaponDefinition.h>
#include <variant>

namespace rwe
{
    struct UnitWeaponStateIdle
    {
    };

    using UnitWeaponAttackTarget = std::variant<UnitId, SimVector>;

    struct UnitWeaponStateAttacking
    {
        struct IdleInfo
        {
        };

        struct AimInfo
        {
            const CobThread* thread;
            SimAngle lastHeading;
            SimAngle lastPitch;
        };

        struct FireInfo
        {
            SimAngle heading;
            SimAngle pitch;

            SimVector targetPosition;

            std::optional<int> firingPiece;

            /** Counts how many shots the weapon has fired so far in the current burst. */
            int burstsFired;

            /** When the next shot of the burst can be fired */
            GameTime readyTime;
        };

        /** The target the weapon is currently trying to shoot at. */
        UnitWeaponAttackTarget target;

        using AttackInfo = std::variant<IdleInfo, AimInfo, FireInfo>;
        AttackInfo attackInfo;

        explicit UnitWeaponStateAttacking(const UnitWeaponAttackTarget& target) : target(target)
        {
        }
    };

    using UnitWeaponState = std::variant<UnitWeaponStateIdle, UnitWeaponStateAttacking>;

    struct UnitWeapon
    {
        std::string weaponType;

        /** The game time at which the weapon next becomes ready to fire. */
        GameTime readyTime{0};

        /** Offset from aim point to firing point, compensation for ballistics calculations. */
        SimScalar ballisticZOffset{0};

        /** The internal state of the weapon. */
        UnitWeaponState state{UnitWeaponStateIdle()};
    };
}
