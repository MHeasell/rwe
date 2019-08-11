#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct CobAngularSpeedTag;
    struct CobAngularSpeed : public OpaqueId<int32_t, CobAngularSpeedTag>
    {
        CobAngularSpeed() = default;
        explicit CobAngularSpeed(ValueType value) : OpaqueId(value) {}

        float toFloat() const
        {
            return static_cast<float>(value) * (360.0f / 65536.0f);
        }
    };
}
