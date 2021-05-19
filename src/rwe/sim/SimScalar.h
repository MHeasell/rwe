#pragma once

#include <rwe/OpaqueField.h>

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

    inline unsigned int simScalarToUInt(SimScalar s)
    {
        return static_cast<unsigned int>(s.value);
    }

    inline SimScalar simScalarFromUInt(unsigned int i)
    {
        return SimScalar(static_cast<float>(i));
    }

    inline SimScalar simScalarFromInt(int i)
    {
        return SimScalar(static_cast<float>(i));
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

    inline SimScalar angularToRadians(SimScalar s)
    {
        return SimScalar((s.value / 65536.0f) * 2.0f * 3.14159265358979323846f);
    }
}
