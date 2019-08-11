#pragma once

#include <functional>

namespace rwe
{
    template <typename T, typename Tag>
    struct OpaqueId
    {
        using ValueType = T;

        ValueType value;

        OpaqueId() = default;
        explicit constexpr OpaqueId(ValueType value) : value(value) {}

        bool operator==(const OpaqueId& rhs) const
        {
            return value == rhs.value;
        }

        bool operator!=(const OpaqueId& rhs) const
        {
            return !(rhs == *this);
        }
    };
}

namespace std
{
    template <typename T, typename Tag>
    struct hash<rwe::OpaqueId<T, Tag>>
    {
        std::size_t operator()(const rwe::OpaqueId<T, Tag>& f) const noexcept
        {
            return std::hash<typename rwe::OpaqueId<T, Tag>::ValueType>()(f.value);
        }
    };
}
