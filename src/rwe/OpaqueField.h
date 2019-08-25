#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    template <typename T, typename Tag>
    struct OpaqueField : public OpaqueId<T, Tag>
    {
        OpaqueField() = default;
        explicit constexpr OpaqueField(T value) : OpaqueId<T, Tag>(value) {}

        bool operator<(OpaqueField<T, Tag> b) const { return this->value < b.value; }
        bool operator>(OpaqueField<T, Tag> b) const { return this->value > b.value; }
        bool operator<=(OpaqueField<T, Tag> b) const { return this->value <= b.value; }
        bool operator>=(OpaqueField<T, Tag> b) const { return this->value >= b.value; }

        OpaqueField<T, Tag> operator+(const OpaqueField<T, Tag>& b) const { return OpaqueField<T, Tag>(this->value + b.value); };
        OpaqueField<T, Tag>& operator+=(const OpaqueField<T, Tag>& b)
        {
            this->value += b.value;
            return *this;
        };
        OpaqueField<T, Tag> operator-() const { return OpaqueField<T, Tag>(-this->value); };
        OpaqueField<T, Tag> operator-(const OpaqueField<T, Tag>& b) const { return OpaqueField<T, Tag>(this->value - b.value); };
        OpaqueField<T, Tag>& operator-=(const OpaqueField<T, Tag>& b)
        {
            this->value -= b.value;
            return *this;
        };
        OpaqueField<T, Tag> operator%(const OpaqueField<T, Tag>& b) const { return OpaqueField<T, Tag>(this->value % b.value); };


        OpaqueField<T, Tag> operator*(const OpaqueField<T, Tag>& b) const { return OpaqueField<T, Tag>(this->value * b.value); };
        OpaqueField<T, Tag>& operator*=(const OpaqueField<T, Tag>& b)
        {
            this->value *= b.value;
            return *this;
        };
        OpaqueField<T, Tag> operator/(const OpaqueField<T, Tag>& b) const { return OpaqueField<T, Tag>(this->value / b.value); };
        OpaqueField<T, Tag>& operator/=(const OpaqueField<T, Tag>& b)
        {
            this->value /= b.value;
            return *this;
        };
    };
}

namespace std
{
    template <typename T, typename Tag>
    struct hash<rwe::OpaqueField<T, Tag>>
    {
        std::size_t operator()(const rwe::OpaqueField<T, Tag>& f) const noexcept
        {
            return std::hash<typename rwe::OpaqueField<T, Tag>::ValueType>()(f.value);
        }
    };
}
