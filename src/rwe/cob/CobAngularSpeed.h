#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct CobAngularSpeedTag;
    struct CobAngularSpeed : public OpaqueId<int32_t, CobAngularSpeedTag>
    {
        CobAngularSpeed() = default;
        explicit CobAngularSpeed(ValueType value) : OpaqueId(value) {}

        SimScalar toSimScalar() const
        {
            return SimScalar(value);
        }
    };
}
