#pragma once

#include <variant>

namespace rwe
{
    struct ProjectilePhysicsTypeLineOfSight
    {
    };
    struct ProjectilePhysicsTypeBallistic
    {
    };
    struct ProjectilePhysicsTypeTracking
    {
        /**
         * Rate at which the projectile turns to face its target in world angular units/tick.
         */
        SimScalar turnRate;
    };

    using ProjectilePhysicsType = std::variant<ProjectilePhysicsTypeLineOfSight, ProjectilePhysicsTypeBallistic, ProjectilePhysicsTypeTracking>;
}
