#pragma once

#include <cstdint>
#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct CobAngularSpeedTag;
    using CobAngularSpeed = OpaqueId<int32_t, CobAngularSpeedTag>;
}
