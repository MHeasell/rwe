#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct PlayerIdTag;
    using PlayerId = OpaqueId<int, PlayerIdTag>;
}
