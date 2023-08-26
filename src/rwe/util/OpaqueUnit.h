#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    template <typename T, typename Tag>
    struct OpaqueUnit : public OpaqueId<T, Tag>
    {
        OpaqueUnit() = default;
        explicit constexpr OpaqueUnit(T value) : OpaqueId<T, Tag>(value) {}

        bool operator<(OpaqueUnit<T, Tag> b) const { return this->value < b.value; }
        bool operator>(OpaqueUnit<T, Tag> b) const { return this->value > b.value; }
        bool operator<=(OpaqueUnit<T, Tag> b) const { return this->value <= b.value; }
        bool operator>=(OpaqueUnit<T, Tag> b) const { return this->value >= b.value; }

        OpaqueUnit<T, Tag> operator+(const OpaqueUnit<T, Tag>& b) const { return OpaqueUnit<T, Tag>(this->value + b.value); };
        OpaqueUnit<T, Tag>& operator+=(const OpaqueUnit<T, Tag>& b)
        {
            this->value += b.value;
            return *this;
        };
        OpaqueUnit<T, Tag> operator-() const { return OpaqueUnit<T, Tag>(-this->value); };
        OpaqueUnit<T, Tag> operator-(const OpaqueUnit<T, Tag>& b) const { return OpaqueUnit<T, Tag>(this->value - b.value); };
        OpaqueUnit<T, Tag>& operator-=(const OpaqueUnit<T, Tag>& b)
        {
            this->value -= b.value;
            return *this;
        };
        OpaqueUnit<T, Tag> operator*(const OpaqueUnit<T, Tag>& b) const { return OpaqueUnit<T, Tag>(this->value * b.value); };
        OpaqueUnit<T, Tag>& operator*=(const OpaqueUnit<T, Tag>& b)
        {
            this->value *= b.value;
            return *this;
        };
        OpaqueUnit<T, Tag> operator%(const OpaqueUnit<T, Tag>& b) const { return OpaqueUnit<T, Tag>(this->value % b.value); };
    };
}

namespace std
{
    template <typename T, typename Tag>
    struct hash<rwe::OpaqueUnit<T, Tag>>
    {
        std::size_t operator()(const rwe::OpaqueUnit<T, Tag>& f) const noexcept
        {
            return std::hash<typename rwe::OpaqueUnit<T, Tag>::ValueType>()(f.value);
        }
    };
}
