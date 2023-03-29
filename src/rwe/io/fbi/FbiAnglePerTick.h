#pragma once

#include <cstdint>
#include <rwe/sim/SimScalar.h>
#include <rwe/util/OpaqueUnit.h>

namespace rwe
{
    struct FbiAnglePerTickTag;
    using FbiAnglePerTick = OpaqueUnit<uint16_t, FbiAnglePerTickTag>;

    inline SimScalar toWorldAnglePerTick(FbiAnglePerTick d)
    {
        return SimScalar(d.value);
    }
}
