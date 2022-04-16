#include "SimScalar.h"

#include <algorithm>
#include <cmath>

namespace rwe
{
    SimScalar rweAbs(SimScalar s)
    {
        return s > 0_ss ? s : -s;
    }

    SimScalar rweSqrt(SimScalar s)
    {
        return SimScalar(std::sqrt(s.value));
    }

    SimScalar rweMax(SimScalar a, SimScalar b)
    {
        return SimScalar(std::max(a.value, b.value));
    }

    SimScalar rweMin(SimScalar a, SimScalar b)
    {
        return SimScalar(std::min(a.value, b.value));
    }

    SimScalar rweAtan2(SimScalar a, SimScalar b)
    {
        return SimScalar(std::atan2(a.value, b.value));
    }
}
