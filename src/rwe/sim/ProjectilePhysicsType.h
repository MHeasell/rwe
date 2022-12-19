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

    using ProjectilePhysicsType = std::variant<ProjectilePhysicsTypeLineOfSight, ProjectilePhysicsTypeBallistic>;
}
