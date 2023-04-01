#pragma once

#include <cstdint>
#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct CobSpeedTag;
    using CobSpeed = OpaqueId<uint32_t, CobSpeedTag>;
}
