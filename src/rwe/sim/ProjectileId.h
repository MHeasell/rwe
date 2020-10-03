#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct ProjectileIdTag;
    using ProjectileId = OpaqueId<unsigned int, ProjectileIdTag>;
}
