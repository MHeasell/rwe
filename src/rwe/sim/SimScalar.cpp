#include "SimScalar.h"

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
}
