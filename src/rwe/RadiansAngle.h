#ifndef RWE_RADIANSANGLE_H
#define RWE_RADIANSANGLE_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct RadiansAngleTag;

    /**
     * Represents an angle in radians in the range -PI <= x < PI.
     */
    struct RadiansAngle : public OpaqueId<float, RadiansAngleTag>
    {
        explicit RadiansAngle(ValueType value);

        RadiansAngle operator-(RadiansAngle rhs);
    };
}

#endif
