#ifndef RWE_UTIL_H
#define RWE_UTIL_H

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <rwe/RadiansAngle.h>
#include <rwe/TaAngle.h>

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

    RadiansAngle toRadians(TaAngle angle);

    TaAngle toTaAngle(RadiansAngle angle);
}

#endif
