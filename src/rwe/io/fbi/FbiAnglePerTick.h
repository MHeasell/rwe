#pragma once

#include <cstdint>
#include <rwe/OpaqueUnit.h>
#include <rwe/sim/SimScalar.h>

namespace rwe
{
    struct FbiAnglePerTickTag;
    using FbiAnglePerTick = OpaqueUnit<uint16_t, FbiAnglePerTickTag>;

    inline SimScalar toWorldAnglePerTick(FbiAnglePerTick d)
    {
        return SimScalar(d.value / 2u);
    }
}
