#include "SimAngle.h"
#include <cmath>
#include <rwe/math/rwe_math.h>
#include <rwe/util.h>

namespace rwe
{
    RadiansAngle toRadians(SimAngle angle)
    {
        return RadiansAngle(wrap(-Pif, Pif, static_cast<float>(angle.value) / 32768.0f * Pif));
    }
    SimAngle fromRadians(RadiansAngle angle)
    {
        return SimAngle(static_cast<uint16_t>(std::round((angle.value / Pif) * 32768.0f)));
    }
    SimAngle atan2(SimScalar a, SimScalar b)
    {
        return fromRadians(RadiansAngle(std::atan2(a.value, b.value)));
    }
    SimScalar hypot(SimScalar a, SimScalar b)
    {
        return SimScalar(std::hypot(a.value, b.value));
    }
    SimScalar cos(SimAngle a)
    {
        return SimScalar(std::cos(toRadians(a).value));
    }
    SimScalar sin(SimAngle a)
    {
        return SimScalar(std::sin(toRadians(a).value));
    }
    SimAngle angleBetween(SimAngle a, SimAngle b)
    {
        auto turn = b - a;
        return (turn.value > HalfTurn.value) ? -turn : turn;
    }

    std::pair<bool, SimAngle> angleBetweenWithDirection(SimAngle a, SimAngle b)
    {
        auto turn = b - a;
        return (turn.value > HalfTurn.value)
            ? std::make_pair(false, -turn)
            : std::make_pair(true, turn);
    }
}
