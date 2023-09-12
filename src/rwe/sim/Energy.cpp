#include "Energy.h"

namespace rwe
{
    Energy rwe::operator*(Energy lhs, SimScalar rhs)
    {
        return Energy(lhs.value * rhs.value);
    }
}
