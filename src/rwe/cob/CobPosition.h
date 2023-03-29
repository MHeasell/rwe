#pragma once

#include <rwe/util/OpaqueId.h>

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

        int toInt() const
        {
            return value / 65536;
        }

        static CobPosition fromFloat(float v)
        {
            return CobPosition(v * 65536.0f);
        }

        static CobPosition fromInt(int v)
        {
            return CobPosition(v * 65536);
        }
    };
}
