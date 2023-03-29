#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct ProjectileIdTag;
    using ProjectileId = OpaqueId<unsigned int, ProjectileIdTag>;
}
