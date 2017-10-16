#ifndef RWE_UTIL_H
#define RWE_UTIL_H

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace rwe
{
    boost::optional<boost::filesystem::path> getSearchPath();
}

#endif
