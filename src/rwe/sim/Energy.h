#pragma once

#include <rwe/sim/SimScalar.h>
#include <rwe/util/OpaqueUnit.h>

namespace rwe
{
    struct EnergyTag;
    using Energy = OpaqueUnit<float, EnergyTag>;

    Energy operator*(Energy lhs, SimScalar rhs);
}
