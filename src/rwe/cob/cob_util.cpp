#include "cob_util.h"
#include <cmath>
#include <rwe/cob/CobPosition.h>
#include <rwe/fixed_point.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    uint32_t cobPackCoords(CobPosition x, CobPosition z)
    {
        auto intX = static_cast<int16_t>(x.toInt());
        auto intZ = static_cast<int16_t>(z.toInt());
        return (static_cast<uint32_t>(intX) << 16) | (static_cast<uint32_t>(intZ) & 0xffff);
    }

    std::pair<CobPosition, CobPosition> cobUnpackCoords(uint32_t xz)
    {
        auto x = CobPosition::fromInt(static_cast<int>(xz >> 16));
        auto z = CobPosition::fromInt(static_cast<int>(xz & 0xffff));
        return std::make_pair(x, z);
    }

    int cobAtan(int a, int b)
    {
        auto result = RadiansAngle::fromUnwrappedAngle(std::atan2(fromFixedPoint(a), fromFixedPoint(b)));
        return static_cast<int>(toCobAngle(result).value);
    }

    CobPosition cobHypot(CobPosition a, CobPosition b)
    {
        return CobPosition::fromFloat(std::hypot(a.toFloat(), b.toFloat()));
    }

    RadiansAngle toRadians(CobAngle angle)
    {
        return RadiansAngle::fromUnwrappedAngle(static_cast<float>(angle.value) * (Pif / 32768.0f));
    }

    CobAngle toCobAngle(RadiansAngle angle)
    {
        return CobAngle(static_cast<uint16_t>(std::round(angle.value * (32768.0f / Pif))));
    }
}
