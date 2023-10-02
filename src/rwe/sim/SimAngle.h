#pragma once

#include <cassert>
#include <cstdint>
#include <rwe/RadiansAngle.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct SimAngleTag;
    using SimAngle = OpaqueId<uint16_t, SimAngleTag>;
    inline SimAngle operator+(const SimAngle& a, const SimAngle& b) { return SimAngle(a.value + b.value); };
    inline SimAngle& operator+=(SimAngle& a, const SimAngle& b)
    {
        a.value += b.value;
        return a;
    };
    inline SimAngle operator-(const SimAngle& a) { return SimAngle(-a.value); };
    inline SimAngle operator-(const SimAngle& a, const SimAngle& b) { return SimAngle(a.value - b.value); };
    inline SimAngle& operator-=(SimAngle& a, const SimAngle& b)
    {
        a.value -= b.value;
        return a;
    };

    constexpr SimAngle MinAngle = SimAngle(0);
    constexpr SimAngle MaxAngle = SimAngle(0xFFFF);

    constexpr SimAngle HalfTurn = SimAngle(1u << 15u);
    constexpr SimAngle QuarterTurn = SimAngle(1u << 14u);
    constexpr SimAngle EighthTurn = SimAngle(1u << 13u);

    RadiansAngle toRadians(SimAngle angle);

    SimAngle fromRadians(RadiansAngle angle);

    inline SimAngle simAngleFromSimScalar(SimScalar s)
    {
        assert(s.value >= 0.0f);
        return SimAngle(static_cast<uint16_t>(s.value));
    }

    SimAngle atan(SimScalar v);

    SimAngle atan2(SimScalar a, SimScalar b);

    SimScalar hypot(SimScalar a, SimScalar b);

    SimScalar cos(SimAngle a);

    SimScalar sin(SimAngle a);

    SimAngle angleBetween(SimAngle a, SimAngle b);

    /**
     * If the returned boolean is true, the angle returned is anticlockwise
     * from a to b. Otherwise, it is clockwise.
     */
    std::pair<bool, SimAngle> angleBetweenWithDirection(SimAngle a, SimAngle b);

    inline bool angleBetweenIsLessOrEqual(SimAngle a, SimAngle b, SimAngle c)
    {
        auto delta = angleBetween(a, b);
        return delta.value <= c.value;
    }

    inline SimAngle turnTowards(SimAngle current, SimAngle target, SimAngle maxTurn)
    {
        auto [anticlockwise, delta] = angleBetweenWithDirection(current, target);

        if (delta.value <= maxTurn.value)
        {
            return target;
        }

        if (anticlockwise)
        {
            return current + maxTurn;
        }
        else
        {
            return current - maxTurn;
        }
    }
}
