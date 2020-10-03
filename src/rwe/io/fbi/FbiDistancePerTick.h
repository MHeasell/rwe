#pragma once

#include <rwe/OpaqueUnit.h>
#include <rwe/sim/SimScalar.h>

namespace rwe
{
    struct FbiDistancePerTickTag;
    using FbiDistancePerTick = OpaqueUnit<float, FbiDistancePerTickTag>;
}
