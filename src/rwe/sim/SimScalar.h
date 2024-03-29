#pragma once

#include <rwe/util/OpaqueField.h>

namespace rwe
{
    struct SimScalarTag;
    using SimScalar = OpaqueField<float, SimScalarTag>;

    constexpr SimScalar operator"" _ss(unsigned long long val)
    {
        return SimScalar(static_cast<float>(val));
    }

    constexpr SimScalar operator"" _ssf(long double val)
    {
        return SimScalar(static_cast<float>(val));
    }

    inline float simScalarToFloat(SimScalar s)
    {
        return static_cast<float>(s.value);
    }

    inline SimScalar floatToSimScalar(float f)
    {
        return SimScalar(f);
    }

    inline SimScalar intToSimScalar(int v)
    {
        return SimScalar(static_cast<float>(v));
    }

    inline unsigned int simScalarToUInt(SimScalar s)
    {
        return static_cast<unsigned int>(s.value);
    }

    inline int roundToInt(SimScalar s)
    {
        return static_cast<int>((s + 0.5_ssf).value);
    }

    inline SimScalar simScalarFromFixed(int f)
    {
        return SimScalar(static_cast<float>(f) / 65536.0f);
    }

    inline int simScalarToFixed(SimScalar f)
    {
        return static_cast<int>(f.value * 65536.0f);
    }

    SimScalar rweAbs(SimScalar s);

    SimScalar rweSqrt(SimScalar s);

    SimScalar rweMax(SimScalar a, SimScalar b);

    SimScalar rweMin(SimScalar a, SimScalar b);

    SimScalar rweAtan2(SimScalar a, SimScalar b);

    SimScalar rweCos(SimScalar v);

    SimScalar rweSin(SimScalar v);

    SimScalar rweAcos(SimScalar v);

    inline SimScalar angularToRadians(SimScalar s)
    {
        return SimScalar((s.value / 65536.0f) * 2.0f * 3.14159265358979323846f);
    }

    inline SimScalar radiansToAngular(SimScalar s)
    {
        return SimScalar(((s.value / 3.14159265358979323846f) / 2.0f) * 65536.0f);
    }
}
