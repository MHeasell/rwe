#pragma once

#include <rwe/sim/SimAngle.h>
#include <rwe/util/OpaqueUnit.h>

namespace rwe
{
    struct CobAngleTag;
    using CobAngle = OpaqueUnit<uint16_t, CobAngleTag>;

    inline SimAngle toWorldAngle(CobAngle angle)
    {
        return SimAngle(angle.value);
    }

    inline CobAngle toCobAngle(SimAngle angle)
    {
        return CobAngle(angle.value);
    }
}
