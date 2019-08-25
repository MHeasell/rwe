#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct GameHashTag;
    using GameHash = OpaqueId<unsigned int, GameHashTag>;
}
