#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct CobSpeedTag;
    struct CobSpeed : public OpaqueId<uint32_t, CobSpeedTag>
    {
        CobSpeed() = default;
        explicit CobSpeed(ValueType value) : OpaqueId(value) {}

        float toFloat() const
        {
            return static_cast<float>(value) / 65536.0f;
        }
    };
}
