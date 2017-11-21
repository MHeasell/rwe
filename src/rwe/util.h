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

    static const float Pif = 3.14159265358979323846f;

    boost::optional<boost::filesystem::path> getLocalDataPath();
    boost::optional<boost::filesystem::path> getSearchPath();

    float toRadians(float v);
}

#endif
