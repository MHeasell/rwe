#pragma once

#include <cstdint>
#include <rwe/OpaqueId.h>
#include <rwe/sim/SimScalar.h>

namespace rwe
{
    struct CobAngularSpeedTag;
    struct CobAngularSpeed : public OpaqueId<int32_t, CobAngularSpeedTag>
    {
        CobAngularSpeed() = default;
        explicit CobAngularSpeed(ValueType value) : OpaqueId(value) {}

        SimScalar toSimScalar() const
        {
            return SimScalar(static_cast<float>(value));
        }
    };
}
