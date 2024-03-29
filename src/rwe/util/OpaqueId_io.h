#pragma once

#include <iostream>
#include <rwe/util/OpaqueId.h>

namespace rwe
{
    template <typename T, typename Tag>
    std::ostream& operator<<(std::ostream& os, const OpaqueId<T, Tag>& v)
    {
        os << v.value;
        return os;
    }

    template <typename T, typename Tag>
    std::istream& operator>>(std::istream& is, OpaqueId<T, Tag>& v)
    {
        is >> v.value;
        return is;
    }
}
