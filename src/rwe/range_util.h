#ifndef RWE_RANGE_UTIL_H
#define RWE_RANGE_UTIL_H

#include <boost/range/adaptors.hpp>

namespace rwe
{
    template <typename Range, typename Chooser>
    auto choose(Range r, Chooser c)
    {
        return r
            | boost::adaptors::transformed(c)
            | boost::adaptors::filtered([](const auto& e) { return e.has_value(); })
            | boost::adaptors::transformed([](const auto& e) { return *e; });
    }
}

#endif
