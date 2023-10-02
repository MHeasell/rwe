#include "Energy.h"

namespace rwe
{
    Energy operator*(Energy lhs, SimScalar rhs)
    {
        return Energy(lhs.value * rhs.value);
    }
}
