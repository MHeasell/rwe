#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct CobPositionTag;
    struct CobPosition : public OpaqueId<int32_t, CobPositionTag>
    {
        CobPosition() = default;
        explicit CobPosition(ValueType value) : OpaqueId(value) {}

        CobPosition operator-() const
        {
            return CobPosition(-value);
        }

        float toFloat() const
        {
            return static_cast<float>(value) / 65536.0f;
        }

        SimScalar toWorldDistance() const
        {
            return SimScalar(toFloat());
        }

        static CobPosition fromWorldDistance(SimScalar d)
        {
            return CobPosition(d.value * 65536.0f);
        }
    };
}
