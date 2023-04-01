#pragma once

#include <cstdint>
#include <rwe/RadiansAngle.h>
#include <rwe/cob/CobAngle.h>
#include <rwe/cob/CobPosition.h>
#include <utility>

namespace rwe
{
    uint32_t cobPackCoords(CobPosition x, CobPosition z);

    std::pair<CobPosition, CobPosition> cobUnpackCoords(uint32_t xz);

    int cobAtan(int a, int b);

    CobPosition cobHypot(CobPosition a, CobPosition b);

    RadiansAngle toRadians(CobAngle angle);

    CobAngle toCobAngle(RadiansAngle angle);
}
