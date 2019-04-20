#ifndef RWE_OPAQUEUNIT_IO_H
#define RWE_OPAQUEUNIT_IO_H

#include <rwe/OpaqueUnit.h>
#include <iostream>

namespace rwe
{
    template <typename T, typename Tag>
    std::ostream& operator<<(std::ostream& os, const OpaqueUnit<T, Tag>& v)
    {
        os << v.value;
        return os;
    }

    template <typename T, typename Tag>
    std::istream& operator>>(std::istream& is, OpaqueUnit<T, Tag>& v)
    {
        is >> v.value;
        return is;
    }
}

#endif
