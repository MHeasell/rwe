#pragma once

#include <cstdint>
#include <rwe/sim/SimScalar.h>
#include <utility>

namespace rwe
{
    uint32_t packCoords(SimScalar x, SimScalar z);

    std::pair<SimScalar, SimScalar> unpackCoords(uint32_t xz);

    int cobAtan(int a, int b);
}
