#include "SimScalar.h"

#include <cmath>

namespace rwe
{
    SimScalar abs(SimScalar s)
    {
        return s > 0_ss ? s : -s;
    }

    SimScalar sqrt(SimScalar s)
    {
        return SimScalar(std::sqrt(s.value));
    }
}
