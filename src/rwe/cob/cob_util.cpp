#include "cob_util.h"
#include <cmath>
#include <rwe/RadiansAngle.h>
#include <rwe/fixed_point.h>
#include <rwe/util.h>

namespace rwe
{
    uint32_t packCoords(SimScalar x, SimScalar z)
    {
        auto intX = static_cast<int16_t>(x.value);
        auto intZ = static_cast<int16_t>(z.value);
        return (static_cast<uint32_t>(intX) << 16) | (static_cast<uint32_t>(intZ) & 0xffff);
    }

    std::pair<SimScalar, SimScalar> unpackCoords(uint32_t xz)
    {
        auto x = static_cast<int16_t>(xz >> 16);
        auto z = static_cast<int16_t>(xz & 0xffff);
        return std::make_pair(SimScalar(x), SimScalar(z));
    }

    int cobAtan(int a, int b)
    {
        auto result = RadiansAngle::fromUnwrappedAngle(std::atan2(fromFixedPoint(a), fromFixedPoint(b)));
        return static_cast<int>(toCobAngle(result).value);
    }
}
