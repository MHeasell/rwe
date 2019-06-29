#ifndef RWE_RANGE_UTIL_H
#define RWE_RANGE_UTIL_H

#include <boost/range/adaptors.hpp>

namespace rwe
{
    template <typename Range, typename Chooser>
    auto choose(Range r, Chooser c)
    {
        return r
            | boost::adaptors::transformed([&](const auto& e) { return c(e); })
            | boost::adaptors::filtered([](const auto& e) { return !!e; })
            | boost::adaptors::transformed([&](const auto& e) { return *e; });
    }
}

#endif
