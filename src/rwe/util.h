#pragma once

#include <boost/filesystem/path.hpp>
#include <optional>
#include <rwe/RadiansAngle.h>
#include <rwe/cob/CobAngle.h>
#include <rwe/float_math.h>

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

    RadiansAngle toRadians(CobAngle angle);

    float toleranceToRadians(unsigned int angle);

    CobAngle toCobAngle(RadiansAngle angle);

    float angleLerp(float a, float b, float t);
}
