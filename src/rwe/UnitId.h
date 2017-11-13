#ifndef RWE_UNITID_H
#define RWE_UNITID_H

#include <functional>

namespace rwe
{
    struct UnitId
    {
        using ValueType = unsigned int;

        ValueType value;

        UnitId() = default;
        explicit UnitId(ValueType value);

        bool operator==(const UnitId& rhs) const;

        bool operator!=(const UnitId& rhs) const;
    };
}

namespace std
{
    template <>
    struct hash<rwe::UnitId>
    {
        std::size_t operator()(const rwe::UnitId& f) const noexcept
        {
            return std::hash<rwe::UnitId::ValueType>()(f.value);
        }
    };
}

#endif
