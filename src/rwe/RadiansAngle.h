#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct RadiansAngleTag;

    /**
     * Represents an angle in radians in the range -PI <= x < PI.
     */
    struct RadiansAngle : public OpaqueId<float, RadiansAngleTag>
    {
        static RadiansAngle fromUnwrappedAngle(float value);

        explicit RadiansAngle(ValueType value);

        RadiansAngle operator-(RadiansAngle rhs);
    };
}
