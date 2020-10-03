#pragma once

#include <rwe/math/Vector3f.h>
#include <rwe/math/Vector3x.h>
#include <rwe/sim/SimAngle.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/util.h>

namespace rwe
{
    using SimVector = Vector3x<SimScalar>;

    inline Vector3f simVectorToFloat(const SimVector& v)
    {
        return Vector3f(simScalarToFloat(v.x), simScalarToFloat(v.y), simScalarToFloat(v.z));
    }

    inline SimVector floatToSimVector(const Vector3f& v)
    {
        return SimVector(floatToSimScalar(v.x), floatToSimScalar(v.y), floatToSimScalar(v.z));
    }
}
