#pragma once

#include <cstdint>
#include <rwe/util/OpaqueUnit.h>

namespace rwe
{
    struct CobAngleTag;
    using CobAngle = OpaqueUnit<uint16_t, CobAngleTag>;
}
