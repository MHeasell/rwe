#include "UnitId.h"

namespace rwe
{
    UnitId::UnitId(ValueType value) : value(value)
    {
    }

    bool UnitId::operator==(const UnitId& rhs) const
    {
        return value == rhs.value;
    }

    bool UnitId::operator!=(const UnitId& rhs) const
    {
        return !(rhs == *this);
    }
}
