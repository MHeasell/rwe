#pragma once

#include <rwe/sim/SimScalar.h>
#include <rwe/util/OpaqueUnit.h>

namespace rwe
{
    struct FbiDistancePerTickTag;
    using FbiDistancePerTick = OpaqueUnit<float, FbiDistancePerTickTag>;
}
