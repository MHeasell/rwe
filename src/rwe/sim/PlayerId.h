#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct PlayerIdTag;
    using PlayerId = OpaqueId<unsigned int, PlayerIdTag>;
}
