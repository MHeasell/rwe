#ifndef RWE_UTIL_H
#define RWE_UTIL_H

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace rwe
{
    enum class Axis
    {
        X,
        Y,
        Z
    };

    boost::optional<boost::filesystem::path> getSearchPath();
}

#endif
