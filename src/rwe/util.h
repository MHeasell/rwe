#pragma once

#include <boost/filesystem/path.hpp>
#include <optional>
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

    std::optional<boost::filesystem::path> getLocalDataPath();
    std::optional<boost::filesystem::path> getSearchPath();

    float toRadians(float v);

    RadiansAngle toRadians(TaAngle angle);

    float toleranceToRadians(unsigned int angle);

    TaAngle toTaAngle(RadiansAngle angle);
}
