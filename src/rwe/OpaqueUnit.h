#ifndef RWE_OPAQUEUNIT_H
#define RWE_OPAQUEUNIT_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    template <typename T, typename Tag>
    struct OpaqueUnitDelta;

    template <typename T, typename Tag>
    struct OpaqueUnit : public OpaqueId<T, Tag>
    {
        OpaqueUnit() = default;
        explicit OpaqueUnit(T value) : OpaqueId<T, Tag>(value) {}

        bool operator<(OpaqueUnit<T, Tag> b) { return this->value < b.value; }
        bool operator>(OpaqueUnit<T, Tag> b) { return this->value > b.value; }
        bool operator<=(OpaqueUnit<T, Tag> b) { return this->value <= b.value; }
        bool operator>=(OpaqueUnit<T, Tag> b) { return this->value >= b.value; }
    };

    template <typename T, typename Tag>
    struct OpaqueUnitDelta : public OpaqueId<T, Tag>
    {
        OpaqueUnitDelta() = default;
        explicit OpaqueUnitDelta(T value) : OpaqueId<T, Tag>(value) {}

        bool operator<(OpaqueUnitDelta<T, Tag> b) { return this->value < b.value; }
        bool operator>(OpaqueUnitDelta<T, Tag> b) { return this->value > b.value; }
        bool operator<=(OpaqueUnitDelta<T, Tag> b) { return this->value <= b.value; }
        bool operator>=(OpaqueUnitDelta<T, Tag> b) { return this->value >= b.value; }
    };

    template <typename T, typename Tag>
    OpaqueUnit<T, Tag> operator+(OpaqueUnit<T, Tag> a, OpaqueUnitDelta<T, Tag> b) { return OpaqueUnit<T, Tag>(a.value + b.value); };

    template <typename T, typename Tag>
    OpaqueUnit<T, Tag> operator-(OpaqueUnit<T, Tag> a, OpaqueUnitDelta<T, Tag> b) { return OpaqueUnit<T, Tag>(a.value - b.value); };

    template <typename T, typename Tag>
    OpaqueUnitDelta<T, Tag> operator-(OpaqueUnit<T, Tag> a, OpaqueUnit<T, Tag> b) { return OpaqueUnitDelta<T, Tag>(a.value - b.value); };
}

#endif
