#ifndef RWE_OPAQUEID_IO_H
#define RWE_OPAQUEID_IO_H

#include <iostream>
#include <rwe/OpaqueId.h>

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

#endif
