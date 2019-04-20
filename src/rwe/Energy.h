#ifndef RWE_ENERGY_H
#define RWE_ENERGY_H

#include <rwe/OpaqueUnit.h>

namespace rwe
{
    struct EnergyTag;
    using Energy = OpaqueUnit<float, EnergyTag>;
}

#endif
